/**
 * \file   libc_backends/l4re_mem/mem.cc
 */
/*
 * (c) 2004-2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#include <l4/re/env>
#include <l4/re/rm>
#include <l4/re/util/cap_alloc>
#include <l4/sys/kdebug.h>

#include <memory>
#include <stdlib.h>
#include <string.h>
#include <mutex>

#include "mem.h"

/**
 * @brief In memory the layout will be as following:
 * At the beginning of each page/dataspace is one DataspaceEntry, containing:
 * - its size
 * - the capability of the next Dataspace
 * - a pointer to the next DataspaceEntry
 *
 * In each dataspace there are Entries, immediately followed by the memory they describe
 * Each entry contains
 * - the size of the memory
 * - the starting address of the memory
 * - a pointer to the next Entry (ordered by addresses)
 * - a pointer to the DataspaceEntry it resides in
 */

bool initialized = false;

constexpr size_t DEFAULT_PAGE_SIZE = 1ul << 16;

static L4::Cap<L4Re::Dataspace> dataspaceCap;

DataspaceEntry* dataspaceAnchor = nullptr;
Entry* entryAnchor = nullptr;

static std::recursive_mutex mtx;

void* DataspaceEntry::endPointer() {
    // the Entry always resides at the beginning of its dataspace
    return (void*)this + size;
}

DataspaceEntry* DataspaceEntry::addDataspace(size_t minSize) {
    // needs to fit at least one dataspace entry at the beginning and one entry
    minSize += sizeof(DataspaceEntry) + sizeof(Entry);
    // bring minsize to a multiple of the default page size
    if (minSize % DEFAULT_PAGE_SIZE != 0) {
        minSize += DEFAULT_PAGE_SIZE - (minSize % DEFAULT_PAGE_SIZE);
    }

    auto size = minSize < DEFAULT_PAGE_SIZE ? DEFAULT_PAGE_SIZE : minSize;

    // find last entry
    auto search = dataspaceAnchor;
    while (search->next != nullptr) {
        search = search->next;
    }

    // get new dataspace
    search->dataspace = L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>();

    if (!search->dataspace.is_valid()) {
        outstring("dataspace is invalid\n");
        return nullptr;
    }

    auto error = L4Re::Env::env()->mem_alloc()->alloc(size, search->dataspace);

    if (error) {
        outstring("error occurred during dataspace allocation\n");
        return nullptr;
    }

    void* addr = 0;

    error = L4Re::Env::env()->rm()->attach(&addr, size, L4Re::Rm::F::Search_addr | L4Re::Rm::F::RW, search->dataspace);

    if (error != 0 || addr == nullptr) {
        outstring("failed to attach a memory region to the dataspace\n");
        return nullptr;
    }

    // make new entry at the beginning of the newly created dataspace
    search->next = (DataspaceEntry*)addr;
    search->next->size = size;
    search->next->next = nullptr;

    return search->next;
}

void DataspaceEntry::removeDataspace(DataspaceEntry* entry) {
    //printDataspaces();

    auto search = dataspaceAnchor;

    if (search == entry) {
        // can't remove the anchor
        outstring("trying to remove dataspace anchor\n");
        return;
    }

    while (search->next != nullptr && search->next != entry) {
        search = search->next;
    }

    if (search->next == nullptr) {
        // couldn't find the dataspace entry to remove
        outstring("did not find dataspace entry to remove\n");
        return;
    }

    // tie pointers
    auto prev = search;
    search = search->next;
    prev->next = search->next;

    // save the dataspace stored in search
    auto tempDataspace = search->dataspace;

    // search is now the dataspaceentry to remove, prev is where the dataspace itself is
    L4Re::Env::env()->rm()->detach(search, nullptr);

    // apparently this is deprecated?
    L4Re::Env::env()->mem_alloc()->free(prev->dataspace);

    // don't forget to move the dataspace
    prev->dataspace = std::move(tempDataspace);
}
void DataspaceEntry::printDataspaces() {
    outstring("[");
    for (auto it = dataspaceAnchor; it != nullptr; it = it->next) {
        outstring("(");
        outhex32(it->size);
        outstring(", ");
        outhex32((l4_uint32_t)(size_t)it->next);
        outstring(")");
        if (it->next != nullptr) {
            outstring(", ");
        }
    }
    outstring("]\n");
}

void* Entry::editEntry(void* addr, size_t newSize) {
    auto search = entryAnchor;
    if (entryAnchor == nullptr) {
        outstring("there are no entries, cannot edit\n");
        return nullptr;
    }

    // find where to edit
    while (search->next != nullptr && search->next->addr != addr) {
        search = search->next;
    }

    if (search->next == nullptr) {
        // couldn't find the entry
        outstring("could not find entry to edit\n");
        return nullptr;
    }

    auto entryptr = search->next;

    // find out how much space is available and if it can be edited in place
    auto endptr = (size_t)entryptr->dataspace->endPointer();
    if (entryptr->next != nullptr && (size_t)entryptr->next->addr < endptr) {
        // there is another entry before the next "addr"
        endptr = (size_t)entryptr->next->addr - sizeof(Entry);
    }
    auto availableSpace = endptr - (size_t)entryptr->addr;
    if (availableSpace >= newSize) {
        // just edit the existing entry
        entryptr->size = newSize;
        return entryptr->addr;
    }

    // otherwise we need to make a new entry, easiest (definitely not most performant) way is to just malloc -> copy -> free
    auto newptr = malloc(newSize);

    memcpy(newptr, entryptr->addr, entryptr->size);

    free(entryptr->addr);

    return newptr;
}

Entry* Entry::insertEntry(void* mem, void* addr, size_t size, DataspaceEntry* ds) {
    auto search = entryAnchor;

    if (search != nullptr) {
        // find where to insert
        while (search->next != nullptr && search->next->addr < addr) {
            search = search->next;
        }
    }
    // insert *after* search

    // just assume that this memory is free and large enough
    auto newEntry = (Entry*)mem;

    newEntry->size = size;
    newEntry->addr = addr;

    if (search != nullptr) {
        // tie up the pointers
        newEntry->next = search->next;
        search->next = newEntry;
    } else {
        entryAnchor = newEntry;
    }

    newEntry->dataspace = ds;

    return newEntry;
}

DataspaceEntry* Entry::removeEntry(void* addr) {
    auto search = entryAnchor;
    if (search == nullptr) {
        outstring("there are no entries, cannot remove\n");
        return nullptr;
    }

    // find where to remove
    while (search->next != nullptr && search->next->addr != addr) {
        search = search->next;
    }

    if (search->next == nullptr) {
        // couldn't find the entry
        outstring("could not find entry to remove\n");
        return nullptr;
    }

    // don't leak stuff
    search->next->addr = nullptr;
    search->next->size = 0;
    auto nextptr = &(search->next->next);

    // find out if any of the neighboring entries are in the same dataspace
    // -> if yes, return null
    // -> if no, return the dataspace so it can be removed
    auto res = search->next->dataspace;
    if (search->dataspace == res) {
        res = nullptr;
    }
    if (search->next->next != nullptr && search->next->next->dataspace == res) {
        res = nullptr;
    }

    search->next = search->next->next;
    *nextptr = nullptr;

    return res;
}

void* Entry::findSpace(size_t size, DataspaceEntry** dataspace) {
    // consider that the entry needs to be stored as well
    size += sizeof(Entry);

    // this assumes that an entry exists in every dataspace
    // empty dataspaces should be immediately freed in free
    auto search = entryAnchor;

    while (search != nullptr) {
        // assume that pages are aligned to their page size
        auto endptr = (size_t)search->dataspace->endPointer();
        if (search->next != nullptr && (size_t)search->next->addr < endptr) {
            // there is another entry before the next "addr"
            endptr = (size_t)search->next->addr - sizeof(Entry);
        }

        auto availableSpace = endptr - (size_t)search->addr - search->size;

        if (availableSpace >= size) {
            *dataspace = search->dataspace;
            return search->addr + search->size;
        }

        search = search->next;
    }

    return nullptr;
}

int init() {
    outstring("init\n");
    entryAnchor = nullptr;
    dataspaceAnchor = nullptr;

    // initialize init memory
    dataspaceCap = L4Re::Util::cap_alloc.alloc<L4Re::Dataspace>();

    if (!dataspaceCap.is_valid()) {
        outstring("dataspace capability is invalid\n");
        return -1;
    }

    auto error = L4Re::Env::env()->mem_alloc()->alloc(DEFAULT_PAGE_SIZE, dataspaceCap);

    if (error) {
        outstring("error occured when allocating the dataspace\n");
        return -1;
    }

    void* addr = 0;
    error = L4Re::Env::env()->rm()->attach(&addr, DEFAULT_PAGE_SIZE, L4Re::Rm::F::Search_addr | L4Re::Rm::F::RW, dataspaceCap);

    if (error != 0 || addr == nullptr) {
        outstring("failed to attach the dataspace to a memory region\n");
    }

    dataspaceAnchor = (DataspaceEntry*)addr;
    dataspaceAnchor->size = DEFAULT_PAGE_SIZE;
    dataspaceAnchor->next = nullptr;

    initialized = true;

    return 0;
}

void* malloc(size_t size) throw() {
    std::lock_guard<std::recursive_mutex> lock(mtx);

    // prevent padding bullshit from fucking stuff up
    if (size % 8 != 0) {
        size += 4 - (size % 4);
    }

    if (!initialized) {
        if (init()) {
            outstring("failed to initialize malloc\n");
            return nullptr;
        }
    }

    DataspaceEntry* dataspace;
    void* memory = Entry::findSpace(size, &dataspace);
    if (memory == nullptr) {
        // if no space was found, get a new page and make a new entry at the beginning of it
        dataspace = DataspaceEntry::addDataspace(size);

        if (dataspace == nullptr) {
            return nullptr;
        }
        memory = (void*)dataspace + sizeof(DataspaceEntry);
    }

    if (dataspace == nullptr) {
        return nullptr;
    }

    // found space -> make new entry and return the address after the entry
    auto ret = Entry::insertEntry(memory, memory + sizeof(Entry), size, dataspace)->addr;
    return ret;
}

void free(void* p) throw() {
    std::lock_guard<std::recursive_mutex> lock(mtx);
    if (!initialized) {
        outstring("trying to free before initialized\n");
        return;
    }

    auto ds = Entry::removeEntry(p);

    if (ds != nullptr) {
        DataspaceEntry::removeDataspace(ds);
    }
}

void* realloc(void* p, size_t size) throw() {
    std::lock_guard<std::recursive_mutex> lock(mtx);
    if (!initialized) {
        outstring("trying to realloc before initialized\n");
        return nullptr;
    }
    return Entry::editEntry(p, size);
}

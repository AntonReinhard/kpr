#pragma once

#include <l4/re/env>
#include <l4/re/rm>
#include <l4/re/util/cap_alloc>
#include <stdlib.h>

struct DataspaceEntry {
    // size of the dataspace
    size_t size;
    // the dataspace capability
    L4::Cap<L4Re::Dataspace> dataspace;

    DataspaceEntry* next;

    void* endPointer();

    /**
     * @brief Will add a dataspace of the given minimum size or larger
     *
     * @param minSize The minimum size that the newly created dataspace should
     * have
     * @return DataspaceEntry* Pointer to the newly created dataspace
     */
    static DataspaceEntry* addDataspace(size_t minSize);

    /**
     * @brief Removes the given dataspace
     *
     * @param entry The entry to remove
     */
    static void removeDataspace(DataspaceEntry* entry);

    /**
     * @brief print the all dataspaces for debug
     *
     */
    static void printDataspaces();
};

/**
 * @brief Node of a double-linked list. Entries are always in order, sorted by
 * their addresses
 *
 */
struct Entry {
    // address of the allocation
    void* addr;

    // pointer to the next entry
    Entry* next;

    // pointer to the dataspace in which this exists
    DataspaceEntry* dataspace;

    // size of the allocation
    // put at the end to save 4 Bytes of padding
    size_t size;

    /**
     * @brief Edits an existing entry to make space for a new size, or make a new entry if not enough space is available where it was
     * 
     * @param addr The address of the memory that the entry that should be edited points to
     * @param newSize The new size the entry should have afterwards
     * @return void* The pointer to the new memory
     */
    static void* editEntry(void* addr, size_t newSize);

    /**
     * @brief Inserts an entry at the correct place so the list stays sorted
     *
     * @param mem The memory where to allocate the new Entry
     * @param addr The address to store in the new Entry
     * @param size The size to store in the new Entry
     * @param ds The dataspace in which this is created
     * @return Entry* The newly created Entry
     */
    static Entry* insertEntry(void* mem, void* addr, size_t size, DataspaceEntry* ds);

    /**
     * @brief Removes the entry at the given address
     *
     * @return The dataspace entry that housed the entry if the last entry in
     * that dataspace was removed, nullptr otherwise or if the entry couldn't be
     * removed
     */
    static DataspaceEntry* removeEntry(void* addr);

    /**
     * @brief Try to find space large enough to fit size in already allocated
     * dataspaces
     *
     * @param size The size to search for
     * @param [out] dataspace The dataspace that the found space lies in
     * @return void* The pointer to create a new entry at, or 0 if not enough
     * space in any of the dataspaces exists
     */
    static void* findSpace(size_t size, DataspaceEntry** dataspace);
};

extern DataspaceEntry* dataspaceAnchor;
extern Entry* entryAnchor;

int init();

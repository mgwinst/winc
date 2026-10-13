#pragma once

#include <vector>
#include <memory>
#include <cstddef>

/*
This allocator is used for simple bump allocation of objects.

This structure lacks any mechanism for shrinking / deallocating arbitrary regions of memory. The only
mechanism for deallocation is deletion of all std::byte[] 'blocks' owned by the allocator.

This allocation scheme is only concerned with raw memory. Object lifetime is handled elsewhere.
*/

// still need to deal with alignment

inline constexpr std::size_t BLOCK_SIZE{ 1<<16 };

struct Block {
    std::unique_ptr<std::byte[]> data;
    std::byte* ptr;
    std::byte* begin;
    std::byte* end;

    Block() :
        data{ std::make_unique<std::byte[]>(BLOCK_SIZE) },
        ptr{ data.get() },
        begin{ data.get() },
        end{ data.get() + BLOCK_SIZE } {}

    void reset()
    {
        ptr = data.get();
        begin = ptr;
        end = ptr;
    }
};

class BumpAllocator
{
private:
    std::vector<Block> blocks;

    Block& current_block()
    {
        return blocks.back();
    }

public:
    BumpAllocator()
    {
        blocks.emplace_back();
    }

    BumpAllocator(const BumpAllocator&) = delete;
    BumpAllocator& operator=(const BumpAllocator&) = delete;
    BumpAllocator(BumpAllocator&&) = default;
    BumpAllocator& operator=(BumpAllocator&&) = default;
    ~BumpAllocator() = default;

    void* allocate(std::size_t n)
    {
        if (current_block().ptr + n > current_block().end) {
            blocks.emplace_back();
        }

        auto* ptr = current_block().ptr;

        current_block().ptr += n;

        return ptr;
    }

    void deallocate()
    {
        if (!blocks.empty())
            blocks.erase(blocks.begin() + 1, blocks.end());

        blocks[0].reset();
    }
};

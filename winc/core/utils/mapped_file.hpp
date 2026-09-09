#pragma once

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <span>
#include <string_view>
#include <print>

class MappedFile
{
public:
    explicit MappedFile(std::string_view file_path)
    {
        int fd = open(file_path.data(), O_RDONLY);
        if (fd == -1) {
            std::println(stderr, "open() failed on file '{}'", file_path);
            exit(1);
        }

        struct stat sb;
        if (fstat(fd, &sb) == -1) {
            close(fd);
            std::println(stderr, "fstat() failed to retrieve file descriptor size");
            exit(1);
        }

        if (sb.st_size <= 0) {
            close(fd);
            std::println(stderr, "file is empty. mmap() will fail");
            exit(1);
        }

        const auto size = static_cast<std::size_t>(sb.st_size);

        void* map = mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (map == MAP_FAILED) {
            close(fd);
            std::println(stderr, "mmap() failed");
            exit(1);
        }

        fd_ = fd;
        buffer_ = map;
        size_ = size;
    }

    ~MappedFile()
    {
        if (buffer_ != MAP_FAILED) {
            munmap(buffer_, size_);
        }

        if (fd_ >= 0) {
            close(fd_);
        }
    }

    std::span<const char> data() const
    {
        return std::span{static_cast<const char*>(buffer_), size_};
    }

private:
    int fd_ = -1;
    void* buffer_ = MAP_FAILED;
    std::size_t size_ = 0;
};

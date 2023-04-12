#include <iostream>
#include <fstream>
#include <vector>
#include <ext2fs/ext2fs.h>

constexpr uint32_t EXT3_SUPER_MAGIC = 0xEF53;
constexpr char MIDI_HEADER[] = {'M', 'T', 'h', 'd'};

bool is_midi_header(const char *data) {
    return std::equal(std::begin(MIDI_HEADER), std::end(MIDI_HEADER), data);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <partition>" << std::endl;
        return 1;
    }

    ext2_filsys fs;
    errcode_t retval;

    retval = ext2fs_open(argv[1], 0, 0, 0, unix_io_manager, &fs);
    if (retval) {
        std::cerr << "Error: Cannot open the partition. Error code: " << retval << std::endl;
        return 1;
    }

    if (fs->super->s_magic != EXT3_SUPER_MAGIC) {
        std::cerr << "Error: Not an ext3 partition." << std::endl;
        return 1;
    }

    int block_size = EXT2_BLOCK_SIZE(fs->super);
    std::vector<char> block_data(block_size);
    int total_blocks = fs->super->s_blocks_count;

    int midi_files_count = 0;

    for (int block = 0; block < total_blocks; ++block) {
        retval = io_channel_read_blk(fs->io, block, 1, block_data.data());
        if (retval) {
            std::cerr << "Error reading block " << block << ". Error code: " << retval << std::endl;
            continue;
        }

        for (int i = 0; i < block_size - (sizeof(MIDI_HEADER) - 1); ++i) {
            if (is_midi_header(block_data.data() + i)) {
                midi_files_count++;
            }
        }
    }

    std::cout << "Found " << midi_files_count << " possible .midi files (including false positives)" << std::endl;

    ext2fs_close(fs);
    return 0;
}
#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <cstring>

constexpr char MIDI_HEADER[] = {'M', 'T', 'h', 'd'};

bool is_midi_header(const char *data) {
    return std::equal(std::begin(MIDI_HEADER), std::end(MIDI_HEADER), data);
}

uint32_t read_big_endian_uint32(const char *data) {
    return (static_cast<uint8_t>(data[0]) << 24) | (static_cast<uint8_t>(data[1]) << 16) |
           (static_cast<uint8_t>(data[2]) << 8) | static_cast<uint8_t>(data[3]);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <partition>" << std::endl;
        return 1;
    }

    std::ifstream partition(argv[1], std::ios::binary);
    if (!partition.is_open()) {
        std::cerr << "Error: Cannot open the partition." << std::endl;
        return 1;
    }

    partition.seekg(0, std::ios::end);
    std::streampos partition_size = partition.tellg();
    partition.seekg(0, std::ios::beg);

    std::vector<char> partition_data(partition_size);
    partition.read(partition_data.data(), partition_size);

    int midi_files_count = 0;

    for (size_t i = 0; i < partition_size - (sizeof(MIDI_HEADER) - 1 + 4); ++i) {
        if (is_midi_header(partition_data.data() + i)) {
            uint32_t midi_size = read_big_endian_uint32(partition_data.data() + i + sizeof(MIDI_HEADER));
            std::cout << "Possible .midi file found at offset " << i << ", size: " << midi_size << " bytes" << std::endl;

            std::string output_filename = "midi_file_" + std::to_string(midi_files_count) + ".midi";
            std::ofstream output_file(output_filename, std::ios::binary);

            if (output_file.is_open()) {
                output_file.write(partition_data.data() + i, midi_size + sizeof(MIDI_HEADER));
                output_file.close();
                std::cout << "Saved potential .midi file as: " << output_filename << std::endl;
                midi_files_count++;
            } else {
                std::cerr << "Error: Cannot create the output file: " << output_filename << std::endl;
            }
        }
    }

    return 0;
}

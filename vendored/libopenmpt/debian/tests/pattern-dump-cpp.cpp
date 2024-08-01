#include <fstream>
#include <iostream>
#include <string>

#include <libopenmpt/libopenmpt.hpp>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <module>" << std::endl;
        return 1;
    }

    try
    {
        // Open input file
        std::ifstream in_file(argv[1], std::ios::in | std::ios::binary);
        if (!in_file.is_open())
        {
            std::cerr << "Error opening input file" << std::endl;
            return 1;
        }

        // Open module
        openmpt::module mod(in_file);

        // Get first pattern number
        if (mod.get_num_orders() == 0)
        {
            std::cerr << "Module has no orders" << std::endl;
            return 1;
        }

        int pattern = mod.get_order_pattern(0);

        // Dump first line of first pattern
        int channels = mod.get_num_channels();
        for (int i = 0; i < channels; i++)
        {
            std::cout << (int) mod.get_pattern_row_channel_command(pattern, 0, i, openmpt::module::command_note);
            std::cout << ' ';
            std::cout << (int) mod.get_pattern_row_channel_command(pattern, 0, i, openmpt::module::command_instrument);
            std::cout << ((i == channels - 1) ? '\n' : ',');
        }

        return 0;
    }
    catch(const openmpt::exception& e)
    {
        std::cerr << "openmpt exception: " << e.what() << std::endl;
        return 1;
    }
}

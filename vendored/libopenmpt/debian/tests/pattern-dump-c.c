#include <stdio.h>

#include <libopenmpt/libopenmpt.h>
#include <libopenmpt/libopenmpt_stream_callbacks_fd.h>
#include <libopenmpt/libopenmpt_stream_callbacks_file.h>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <module>\n", argv[0]);
        return 1;
    }

    // Open input file
    FILE* in_file = fopen(argv[1], "rb");
    if (in_file == NULL)
    {
        perror("Error opening input file");
        return 1;
    }

    // Open module
    openmpt_module* mod = openmpt_module_create(
            openmpt_stream_get_file_callbacks(),
            in_file,
            openmpt_log_func_default,
            NULL,
            NULL);

    fclose(in_file);
    if (mod == NULL)
    {
        fputs("Error reading module", stderr);
        return 1;
    }

    // Get first pattern number
    if (openmpt_module_get_num_orders(mod) == 0)
    {
        fputs("Module has no orders", stderr);
        return 1;
    }

    int pattern = openmpt_module_get_order_pattern(mod, 0);

    // Dump first line of first pattern
    int channels = openmpt_module_get_num_channels(mod);
    for (int i = 0; i < channels; i++)
    {
        printf("%d %d%c",
                openmpt_module_get_pattern_row_channel_command(mod, pattern, 0, i, OPENMPT_MODULE_COMMAND_NOTE),
                openmpt_module_get_pattern_row_channel_command(mod, pattern, 0, i, OPENMPT_MODULE_COMMAND_INSTRUMENT),
                (i == channels - 1) ? '\n' : ',');
    }

    openmpt_module_destroy(mod);
    return 0;
}

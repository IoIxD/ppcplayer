#include <stdio.h>
#include <stdlib.h>

#include <libmodplug/modplug.h>

struct var_buffer
{
    void* buf;
    int length;
};

// Read entire file into buffer
//  buf is NULL on failure
struct var_buffer read_file(const char* filename)
{
    struct var_buffer result = { NULL, 0 };
    FILE* in_file = fopen(filename, "rb");
    if (in_file == NULL)
    {
        perror("Error opening input file");
        return result;
    }

    // Get file size
    fseek(in_file, 0, SEEK_END);
    int size = ftell(in_file);
    rewind(in_file);

    // Allocate buffer and read data into it
    void* buf = malloc(size);
    if (buf == NULL)
    {
        fputs("Error allocating memory", stderr);
        goto done;
    }

    if (fread(buf, size, 1, in_file) != 1)
    {
        perror("Error reading file");
        goto done;
    }

    result.buf = buf;
    result.length = size;

done:
    fclose(in_file);
    return result;
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <module>\n", argv[0]);
        return 1;
    }

    // Read entire input file into buffer
    struct var_buffer buffer = read_file(argv[1]);
    if (buffer.buf == NULL)
        return 1;

    // Load module
    ModPlugFile* mod = ModPlug_Load(buffer.buf, buffer.length);
    if (mod == NULL)
    {
        fputs("Error loading module", stderr);
        free(buffer.buf);
        return 1;
    }

    // Get first pattern number
    ModPlug_SeekOrder(mod, 0);
    int pattern = ModPlug_GetCurrentPattern(mod);

    // Dump first line of first pattern
    int channels = ModPlug_NumChannels(mod);
    const ModPlugNote* note_data = ModPlug_GetPattern(mod, pattern, NULL);

    for (int i = 0; i < channels; i++)
    {
        printf("%d %d%c",
                note_data[i].Note,
                note_data[i].Instrument,
                (i == channels - 1) ? '\n' : ',');
    }

    ModPlug_Unload(mod);
    free(buffer.buf);
    return 0;
}

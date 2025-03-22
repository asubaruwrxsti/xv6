#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define FDT_MAGIC 0xd00dfeed
#define FDT_BEGIN_NODE 0x1
#define FDT_END_NODE 0x2
#define FDT_PROP 0x3
#define FDT_NOP 0x4
#define FDT_END 0x9

struct fdt_header
{
    uint32_t magic;
    uint32_t totalsize;
    uint32_t off_dt_struct;
    uint32_t off_dt_strings;
    uint32_t off_mem_rsvmap;
    uint32_t version;
    uint32_t last_comp_version;
    uint32_t boot_cpuid_phys;
    uint32_t size_dt_strings;
    uint32_t size_dt_struct;
};

static uint32_t fdt32_to_cpu(uint32_t x)
{
    return __builtin_bswap32(x);
}

static uint64_t fdt64_to_cpu(uint64_t x)
{
    return __builtin_bswap64(x);
}

void parse_memory_info(const void *fdt)
{
    const struct fdt_header *header = (const struct fdt_header *)fdt;

    // Verify the magic number
    if (fdt32_to_cpu(header->magic) != FDT_MAGIC)
    {
        fprintf(stderr, "Error: Invalid device tree blob\n");
        return;
    }

    const char *dt_struct = (const char *)fdt + fdt32_to_cpu(header->off_dt_struct);
    const char *dt_strings = (const char *)fdt + fdt32_to_cpu(header->off_dt_strings);

    const char *p = dt_struct;
    const char *end = dt_struct + fdt32_to_cpu(header->size_dt_struct);

    int in_memory_node = 0;

    while (p < end)
    {
        uint32_t token = fdt32_to_cpu(*(uint32_t *)p);
        p += 4;

        switch (token)
        {
        case FDT_BEGIN_NODE:
        {
            const char *name = p;
            size_t name_len = strlen(name) + 1;
            p += (name_len + 3) & ~3; // Align to 4 bytes

            if (strcmp(name, "memory") == 0)
            {
                in_memory_node = 1;
            }
            break;
        }
        case FDT_END_NODE:
        {
            in_memory_node = 0;
            break;
        }
        case FDT_PROP:
        {
            uint32_t len = fdt32_to_cpu(*(uint32_t *)p);
            uint32_t nameoff = fdt32_to_cpu(*(uint32_t *)(p + 4));
            const char *prop_name = dt_strings + nameoff;
            const void *prop_data = p + 8;

            if (in_memory_node && strcmp(prop_name, "reg") == 0)
            {
                if (len >= 16)
                { // At least two 64-bit values
                    uint64_t base = fdt64_to_cpu(*(uint64_t *)prop_data);
                    uint64_t size = fdt64_to_cpu(*(uint64_t *)(prop_data + 8));
                    printf("Physical memory base: 0x%llx\n", base);
                    printf("Physical memory size: %llu bytes (%.2f MB)\n", size, size / (1024.0 * 1024.0));
                }
                else
                {
                    fprintf(stderr, "Error: 'reg' property is too short\n");
                }
            }

            p += 8 + ((len + 3) & ~3); // Align to 4 bytes
            break;
        }
        case FDT_NOP:
            break;
        case FDT_END:
            return;
        default:
            fprintf(stderr, "Error: Unknown token 0x%x\n", token);
            return;
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <device_tree_blob>\n", argv[0]);
        return 1;
    }

    const char *dtb_path = argv[1];
    void *fdt;
    FILE *file;
    long file_size;

    // Open the device tree blob file
    file = fopen(dtb_path, "rb");
    if (!file)
    {
        perror("Error opening device tree blob file");
        return 1;
    }

    // Get the file size
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory and read the file
    fdt = malloc(file_size);
    if (!fdt)
    {
        fprintf(stderr, "Error: Could not allocate memory for device tree blob\n");
        fclose(file);
        return 1;
    }
    fread(fdt, 1, file_size, file);
    fclose(file);

    // Parse the memory information
    parse_memory_info(fdt);

    // Free allocated memory
    free(fdt);
    return 0;
}
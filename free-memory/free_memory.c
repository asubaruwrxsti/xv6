// This uses the Mach kernel interface to get the amount of free memory available on macOS.

#include <stdio.h>

// System control interface headers
#include <sys/sysctl.h>

// Mach kernel interface headers needed for memory statistics
#include <mach/mach.h>
#include <mach/vm_statistics.h>
#include <mach/mach_types.h>
#include <mach/mach_init.h>
#include <mach/mach_host.h>

uint64_t get_free_memory(void)
{
    // Page size of memory - typically 4096 bytes on macOS
    vm_size_t page_size;

    // Port to the Mach host - needed for system information calls
    mach_port_t mach_port;

    // Count of information elements returned by host_statistics64
    mach_msg_type_number_t count;

    // Structure to store virtual memory statistics
    vm_statistics64_data_t vm_stats;

    // Get a port to the Mach host
    mach_port = mach_host_self();
    count = sizeof(vm_stats) / sizeof(natural_t);

    // Get the system's page size
    if (host_page_size(mach_port, &page_size) != KERN_SUCCESS)
    {
        return 0;
    }

    // Get virtual memory statistics from the host
    // HOST_VM_INFO64 provides 64-bit VM statistics
    if (host_statistics64(mach_port, HOST_VM_INFO64, (host_info64_t)&vm_stats, &count) != KERN_SUCCESS)
    {
        return 0;
    }

    // Calculate total free memory:
    // Multiply the number of free pages by the page size
    // vm_stats.free_count represents the number of free pages in the system
    // Pages that are:
    // - not in use by any process
    // - not used as file cache
    // - not used as buffer cache
    uint64_t free_memory = (uint64_t)vm_stats.free_count * (uint64_t)page_size;
    return free_memory;
}

int main()
{
    uint64_t free_mem = get_free_memory();
    printf("Free memory available: %llu bytes (%.2f GB)\n",
           free_mem,
           (double)free_mem / (1024 * 1024 * 1024));
    return 0;
}
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <libbladeRF.h>

#define NUM_FREQUENCIES 12
#define ITERATIONS 1000
#define BASE_FREQUENCY 3100000000

int example(struct bladerf *dev, bladerf_module module)
{
    /** [example] */
    int status;
    unsigned int i, j;

    unsigned int frequencies[NUM_FREQUENCIES] = {
        902000000,
        902100000,
        902200000,
        902300000,
        902400000,
        902500000,
        902500000,
        902600000,
        902700000,
        902800000,
        902900000,
        903000000
    };


    struct bladerf_quick_tune quick_tunes[NUM_FREQUENCIES];

    /* Get our quick tune parameters for each frequency we'll be using */
    for (i = 0; i < NUM_FREQUENCIES; i++) {
        status = bladerf_set_frequency(dev, module, frequencies[i]);
        if (status != 0) {
            fprintf(stderr, "Failed to set frequency to %u Hz: %s\n",
                    frequencies[i], bladerf_strerror(status));
            return status;
        }

        status = bladerf_get_quick_tune(dev, module, &quick_tunes[i]);
        if (status != 0) {
            fprintf(stderr, "Failed to get quick tune for %u Hz: %s\n",
                    frequencies[i], bladerf_strerror(status));
            return status;
        }
    }

    for (i = j = 0; i < ITERATIONS; i++) {
        /* Tune to the specified frequency immediately via BLADERF_RETUNE_NOW.
         *
         * Alternatively, this re-tune could be scheduled by providing a
         * timestamp counter value */
        status = bladerf_schedule_retune(dev, module, BLADERF_RETUNE_NOW, 0,
                                         &quick_tunes[j]);

        if (status != 0) {
            fprintf(stderr, "Failed to apply quick tune: %s\n",
                    bladerf_strerror(status));
            return status;
        }

        j = (j + 1) % NUM_FREQUENCIES;

        /* ... Handle signals at current frequency ... */

    }

    /** [example] */

    return status;
}

int main(int argc, char *argv[])
{
    int status = 0;

    struct bladerf *dev = NULL;
    struct bladerf_devinfo dev_info;

    /* Initialize the information used to identify the desired device
     * to all wildcard (i.e., "any device") values */
    bladerf_init_devinfo(&dev_info);

    /* Request a device with the provided serial number.
     * Invalid strings should simply fail to match a device. */
    if (argc >= 2) {
        fprintf(stdout, "dev_info.serial: %s", dev_info.serial);
        strncpy(dev_info.serial, argv[1], sizeof(dev_info.serial) - 1);
    }

    status = bladerf_open_with_devinfo(&dev, &dev_info);
    if (status != 0) {
        fprintf(stderr, "Unable to open device: %s\n",
                bladerf_strerror(status));
        return 1;
    }

    /* A quick check that this works is to watch LO leakage on a VSA */

    status = bladerf_enable_module(dev, BLADERF_MODULE_TX, true);
    if (status != 0) {
        fprintf(stderr, "Failed to enable TX module: %s\n",
                bladerf_strerror(status));
        return status;
    }

    status = example(dev, BLADERF_MODULE_TX);

    bladerf_enable_module(dev, BLADERF_MODULE_TX, false);
    bladerf_close(dev);
    return status;
}

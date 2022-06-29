
#include "resources.h"

/* ==== Private typedefs ==== */

static const char *TAG = "resources";

typedef struct rsrc rsrc_t;

struct rsrc {
    /* ==== Linked list ==== */
    // Previous resource in the linked list, if any.
    rsrc_t     *prev;
    // Next resource in the linked list, if any.
    rsrc_t     *next;
    /* ==== Loaded status ==== */
    // Buffer containing the resource data.
    pax_buf_t  *buf;
    // Whether the resource is loaded.
    bool        loaded;
    // Whether the resource needs to stay loaded long-term.
    bool        long_term;
    /* ==== Location ==== */
    // The filename of the resource.
    char       *filename;
    // Whether the resource is builtin.
    bool        builtin;
    // The start location of builtin resources.
    const void *start;
    // The end location of builtin resources.
    const void *end;
};



/* ==== Resource data ==== */

#define RSRC_BUILTIN_PNG(name, id) (rsrc_t) {\
        .prev      = NULL,\
        .next      = NULL,\
        .buf       = NULL,\
        .loaded    = false,\
        .long_term = false,\
        .filename  = name,\
        .builtin   = true,\
        .start     = id ## _start,\
        .end       = id ## _end,\
    }

extern const uint8_t rsrc_dust_start[] asm("_binary_dust_png_start");
extern const uint8_t rsrc_dust_end[]   asm("_binary_dust_png_end");

static rsrc_t builtins[] = {
    RSRC_BUILTIN_PNG("dust.png", rsrc_dust),
};
static const size_t num_builtins = sizeof(builtins) / sizeof(rsrc_t);



// Find the location of and load a resource.
static pax_buf_t *resource_load(rsrc_t *rsrc) {
    if (rsrc->loaded) return rsrc->buf;
    if (rsrc->start && rsrc->end) {
        // Load from embedded data.
        // Make buffer.
        rsrc->buf = malloc(sizeof(pax_buf_t));
        if (!rsrc->buf) return NULL;
        // Decode PNG.
        bool success = pax_decode_png_buf(
            rsrc->buf, rsrc->start, rsrc->end-rsrc->start,
            PAX_BUF_32_8888ARGB, CODEC_FLAG_OPTIMAL
        );
        if (!success) {
            // Decode error.
            free(rsrc->buf);
            rsrc->buf = NULL;
        } else {
            // Decode success.
            rsrc->loaded = true;
            ESP_LOGI(TAG, "Loaded '%s'.", rsrc->filename);
        }
    } else {
        // There isn't anything here.
        rsrc->loaded = false;
    }
    return rsrc->buf;
}

// Unload and free memory associated with a resource.
static void resource_unload(rsrc_t *rsrc) {
    if (rsrc->loaded) {
        // Mark unloaded.
        rsrc->loaded = false;
        // Destroy buffer.
        pax_buf_destroy(rsrc->buf);
        // Free buffer struct.
        free(rsrc->buf);
        rsrc->buf = NULL;
        ESP_LOGI(TAG, "Unloaded '%s'.", rsrc->filename);
    }
}

// Look for an embedded resource.
static rsrc_t *resource_find(const char *filename) {
    // Sift through embedded resources.
    for (size_t i = 0; i < num_builtins; i++) {
        if (!strcmp(builtins[i].filename, filename)) {
            // Match found.
            return &builtins[i];
        }
    }
    // No match found.
    return NULL;
}

// Get a resource that needs to be available for a long time.
pax_buf_t *resource_get_long(const char *filename) {
    rsrc_t *rsrc = resource_find(filename);
    if (rsrc) {
        rsrc->long_term = true;
        return resource_load(rsrc);
    } else {
        return NULL;
    }
}

// Get a resource that needs to be available for a short time is called.
pax_buf_t *resource_get(const char *filename) {
    rsrc_t *rsrc = resource_find(filename);
    if (rsrc) {
        return resource_load(rsrc);
    } else {
        return NULL;
    }
}

// Promises that loaded resources won't be used again until explicitly asked for.
void resource_mark_frame() {
    
}

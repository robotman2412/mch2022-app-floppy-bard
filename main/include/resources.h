
#pragma once

#include "types.h"
#include "pax_codecs.h"
#include "string.h"

// Get a resource that needs to be available for a long time.
pax_buf_t *resource_get_long(const char *filename);
// Get a resource that needs to be available until resource_mark_frame is called.
pax_buf_t *resource_get(const char *filename);
// Promises that loaded resources won't be used again until explicitly asked for.
void resource_mark_frame();

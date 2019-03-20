/*
 * Copyright (C) 2018 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "OpenglRender/render_api_types.h"

#ifdef _MSC_VER
# ifdef BUILDING_EMUGL_COMMON_SHARED
#  define EMUGL_COMMON_API __declspec(dllexport)
# else
#  define EMUGL_COMMON_API __declspec(dllimport)
#endif
#else
# define EMUGL_COMMON_API
#endif

namespace emugl {
EMUGL_COMMON_API extern emugl_counter_t emugl_object_count_inc;
EMUGL_COMMON_API extern emugl_counter_t emugl_object_count_dec;
EMUGL_COMMON_API void set_emugl_gl_object_counter(emugl_counter_struct counter);
}
#define GL_OBJECT_COUNT_INC(TYPE)       \
    do {                                \
        emugl::emugl_object_count_inc((TYPE)); \
    } while (0)

#define GL_OBJECT_COUNT_DEC(TYPE)       \
    do {                                \
        emugl::emugl_object_count_dec((TYPE)); \
    } while (0)
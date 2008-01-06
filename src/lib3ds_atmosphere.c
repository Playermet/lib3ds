/*
 * The 3D Studio File Format Library
 * Copyright (C) 1996-2007 by Jan Eric Kyprianidis <www.kyprianidis.com>
 * All rights reserved.
 *
 * This program is  free  software;  you can redistribute it and/or modify it
 * under the terms of the  GNU Lesser General Public License  as published by
 * the  Free Software Foundation;  either version 2.1 of the License,  or (at
 * your option) any later version.
 *
 * This  program  is  distributed in  the  hope that it will  be useful,  but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or  FITNESS FOR A  PARTICULAR PURPOSE.  See the  GNU Lesser General Public
 * License for more details.
 *
 * You should  have received  a copy of the GNU Lesser General Public License
 * along with  this program;  if not, write to the  Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Id: atmosphere.c,v 1.12 2007/06/20 17:04:08 jeh Exp $
 */
#include "lib3ds_impl.h"


static void
fog_read(Lib3dsAtmosphere *at, Lib3dsIo *io) {
    Lib3dsChunk c;
    Lib3dsWord chunk;

    lib3ds_chunk_read_start(&c, LIB3DS_FOG, io);

    at->fog.near_plane = lib3ds_io_read_float(io);
    at->fog.near_density = lib3ds_io_read_float(io);
    at->fog.far_plane = lib3ds_io_read_float(io);
    at->fog.far_density = lib3ds_io_read_float(io);
    lib3ds_chunk_read_tell(&c, io);

    while ((chunk = lib3ds_chunk_read_next(&c, io)) != 0) {
        switch (chunk) {
            case LIB3DS_LIN_COLOR_F: {
                int i;
                for (i = 0; i < 3; ++i) {
                    at->fog.col[i] = lib3ds_io_read_float(io);
                }
            }
            break;

            case LIB3DS_COLOR_F:
                break;

            case LIB3DS_FOG_BGND: {
                at->fog.fog_background = TRUE;
            }
            break;

            default:
                lib3ds_chunk_unknown(chunk, io);
        }
    }

    lib3ds_chunk_read_end(&c, io);
}


static void
layer_fog_read(Lib3dsAtmosphere *at, Lib3dsIo *io) {
    Lib3dsChunk c;
    Lib3dsWord chunk;
    Lib3dsBool have_lin = FALSE;

    lib3ds_chunk_read_start(&c, LIB3DS_LAYER_FOG, io);

    at->layer_fog.near_y = lib3ds_io_read_float(io);
    at->layer_fog.far_y = lib3ds_io_read_float(io);
    at->layer_fog.density = lib3ds_io_read_float(io);
    at->layer_fog.flags = lib3ds_io_read_dword(io);
    lib3ds_chunk_read_tell(&c, io);

    while ((chunk = lib3ds_chunk_read_next(&c, io)) != 0) {
        switch (chunk) {
            case LIB3DS_LIN_COLOR_F:
                lib3ds_io_read_rgb(io, at->layer_fog.col);
                have_lin = TRUE;
                break;

            case LIB3DS_COLOR_F:
                lib3ds_io_read_rgb(io, at->layer_fog.col);
                break;

            default:
                lib3ds_chunk_unknown(chunk, io);
        }
    }

    lib3ds_chunk_read_end(&c, io);
}


static void
distance_cue_read(Lib3dsAtmosphere *at, Lib3dsIo *io) {
    Lib3dsChunk c;
    Lib3dsWord chunk;

    lib3ds_chunk_read_start(&c, LIB3DS_DISTANCE_CUE, io);

    at->dist_cue.near_plane = lib3ds_io_read_float(io);
    at->dist_cue.near_dimming = lib3ds_io_read_float(io);
    at->dist_cue.far_plane = lib3ds_io_read_float(io);
    at->dist_cue.far_dimming = lib3ds_io_read_float(io);
    lib3ds_chunk_read_tell(&c, io);

    while ((chunk = lib3ds_chunk_read_next(&c, io)) != 0) {
        switch (chunk) {
            case LIB3DS_DCUE_BGND: {
                at->dist_cue.cue_background = TRUE;
            }
            break;

            default:
                lib3ds_chunk_unknown(chunk, io);
        }
    }

    lib3ds_chunk_read_end(&c, io);
}


void
lib3ds_atmosphere_read(Lib3dsAtmosphere *atmosphere, Lib3dsIo *io) {
    Lib3dsChunk c;

    lib3ds_chunk_read(&c, io);
    switch (c.chunk) {
        case LIB3DS_FOG: {
            lib3ds_chunk_read_reset(&c, io);
            fog_read(atmosphere, io);
            break;
        }

        case LIB3DS_LAYER_FOG: {
            lib3ds_chunk_read_reset(&c, io);
            layer_fog_read(atmosphere, io);
            break;
        }

        case LIB3DS_DISTANCE_CUE: {
            lib3ds_chunk_read_reset(&c, io);
            distance_cue_read(atmosphere, io);
            break;
        }

        case LIB3DS_USE_FOG: {
            atmosphere->fog.use = TRUE;
            break;
        }

        case LIB3DS_USE_LAYER_FOG: {
            atmosphere->layer_fog.use = TRUE;
            break;
        }

        case LIB3DS_USE_DISTANCE_CUE: {
            atmosphere->dist_cue.use = TRUE;
            break;
        }
    }
}


void
lib3ds_atmosphere_write(Lib3dsAtmosphere *atmosphere, Lib3dsIo *io) {
    if (atmosphere->fog.use) { /*---- LIB3DS_FOG ----*/
        Lib3dsChunk c;
        c.chunk = LIB3DS_FOG;
        lib3ds_chunk_write_start(&c, io);

        lib3ds_io_write_float(io, atmosphere->fog.near_plane);
        lib3ds_io_write_float(io, atmosphere->fog.near_density);
        lib3ds_io_write_float(io, atmosphere->fog.far_plane);
        lib3ds_io_write_float(io, atmosphere->fog.far_density);
        {
            Lib3dsChunk c;
            c.chunk = LIB3DS_COLOR_F;
            c.size = 18;
            lib3ds_chunk_write(&c, io);
            lib3ds_io_write_rgb(io, atmosphere->fog.col);
        }
        if (atmosphere->fog.fog_background) {
            Lib3dsChunk c;
            c.chunk = LIB3DS_FOG_BGND;
            c.size = 6;
            lib3ds_chunk_write(&c, io);
        }

        lib3ds_chunk_write_end(&c, io);
    }

    if (atmosphere->layer_fog.use) { /*---- LIB3DS_LAYER_FOG ----*/
        Lib3dsChunk c;
        c.chunk = LIB3DS_LAYER_FOG;
        c.size = 40;
        lib3ds_chunk_write(&c, io);
        lib3ds_io_write_float(io, atmosphere->layer_fog.near_y);
        lib3ds_io_write_float(io, atmosphere->layer_fog.far_y);
        lib3ds_io_write_float(io, atmosphere->layer_fog.near_y);
        lib3ds_io_write_dword(io, atmosphere->layer_fog.flags);
        {
            Lib3dsChunk c;
            c.chunk = LIB3DS_COLOR_F;
            c.size = 18;
            lib3ds_chunk_write(&c, io);
            lib3ds_io_write_rgb(io, atmosphere->fog.col);
        }
    }

    if (atmosphere->dist_cue.use) { /*---- LIB3DS_DISTANCE_CUE ----*/
        Lib3dsChunk c;
        c.chunk = LIB3DS_DISTANCE_CUE;
        lib3ds_chunk_write_start(&c, io);

        lib3ds_io_write_float(io, atmosphere->dist_cue.near_plane);
        lib3ds_io_write_float(io, atmosphere->dist_cue.near_dimming);
        lib3ds_io_write_float(io, atmosphere->dist_cue.far_plane);
        lib3ds_io_write_float(io, atmosphere->dist_cue.far_dimming);
        if (atmosphere->dist_cue.cue_background) {
            Lib3dsChunk c;
            c.chunk = LIB3DS_DCUE_BGND;
            c.size = 6;
            lib3ds_chunk_write(&c, io);
        }

        lib3ds_chunk_write_end(&c, io);
    }

    if (atmosphere->fog.use) { /*---- LIB3DS_USE_FOG ----*/
        Lib3dsChunk c;
        c.chunk = LIB3DS_USE_FOG;
        c.size = 6;
        lib3ds_chunk_write(&c, io);
    }

    if (atmosphere->layer_fog.use) { /*---- LIB3DS_USE_LAYER_FOG ----*/
        Lib3dsChunk c;
        c.chunk = LIB3DS_USE_LAYER_FOG;
        c.size = 6;
        lib3ds_chunk_write(&c, io);
    }

    if (atmosphere->dist_cue.use) { /*---- LIB3DS_USE_DISTANCE_CUE ----*/
        Lib3dsChunk c;
        c.chunk = LIB3DS_USE_V_GRADIENT;
        c.size = 6;
        lib3ds_chunk_write(&c, io);
    }
}


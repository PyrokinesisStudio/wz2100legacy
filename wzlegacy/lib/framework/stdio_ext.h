/*This code copyrighted (2013) for the Warzone 2100 Legacy Project under the GPLv2.

Warzone 2100 Legacy is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

Warzone 2100 Legacy is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Warzone 2100 Legacy; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA*/
#ifndef STDIO_EXT_H
#define STDIO_EXT_H

#include <stdarg.h>

#if defined(__cplusplus)
extern "C" {
#endif

    /** A variant on snprintf which appends its output string to the given string
     *  buffer, rather than to replace it.
     *  \param str the string to append to
     *  \param size the size of the buffer \c str expressed in bytes
     *  \param format the formatting string
     *  \param ap a variable arguments list of variables to use in the formatting
     *            string
     *  \return the amount of characters appended to the string
     */
    extern int vslcatprintf(char *str, size_t size, const char *format, va_list ap);


    /** A variant on snprintf which appends its output string to the given string
     *  The function's interface is similar to vslcatprintf(), so look at that
     *  function's description.
     */
    extern int slcatprintf(char *str, size_t size, const char *format, ...) WZ_DECL_FORMAT(printf, 3, 4);


#if defined(WZ_OS_WIN) || defined(DOXYGEN)
// These functions are GNU extensions; so make sure they are available on Windows also

    /**
     * This function is analogue to vsprintf, except that it allocates a string
     * large enough to hold the output including the terminating NUL character.
     *
     * \param[out] strp   If successful will hold a pointer to the formatted
     *                    string. This pointer should be passed to free() to
     *                    release the allocated storage when it is no longer
     *                    needed. When unsuccessful the contents of \c strp are
     *                    undefined (no memory will have been allocated though).
     *
     * \param      format The format specifier (the same to vsprintf).
     *
     * \param      ap     An argument list to be used in combination with \c format
     *                    to print the string.
     *
     * \return When successful the amount of characters printed, just like
     *         vsprintf. If memory allocation wasn't possible or some other error
     *         occurred, -1 is returned.
     */
    extern int vasprintf(char **strp, const char *format, va_list ap);


    /**
     * This function is analogue to sprintf, except that it allocates a string
     * large enough to hold the output including the terminating NUL character.
     *
     * @see vasprintf()
     */
    extern int asprintf(char **strp, const char *format, ...) WZ_DECL_FORMAT(printf, 2, 3);
#endif

#if defined(WZ_CC_MSVC)
// Make sure that these functions are available, and work according to the C99 spec on MSVC also

    extern int wz_vsnprintf(char *str, size_t size, const char *format, va_list ap);
    extern int wz_snprintf(char *str, size_t size, const char *format, ...);

// Necessary to prevent conflicting symbols with MSVC's own (incorrect!) implementations of these functions
# define vsnprintf wz_vsnprintf
# define snprintf  wz_snprintf
#elif defined(__cplusplus) && defined(WZ_CC_GNU)
// Do nothing here, and assume that G++ has a proper implementation of snprintf and vsnprintf
#elif !defined(WZ_CC_GNU) && !defined(WZ_C99)
# error "This code depends on a C99-compliant implementation of snprintf and vsnprintf; please compile as C99 or provide a compliant implementation!"
#endif


// A stack-allocating variant of sprintf
#define sasprintf(strp, format, ...) \
do { \
	/* Make sure to evaluate "format" just once */ \
	const char* fmt = format; \
	/* Determine the size of the string we're going to produce */ \
	size_t size = snprintf(NULL, 0, fmt, __VA_ARGS__); \
	\
	/* Let the compiler perform some static type-checking */ \
	char** var = strp; \
	\
	/* Allocate a buffer large enough to hold our string on the stack*/ \
	*var = (char*)alloca(size + 1); \
	/* Print into our newly created string-buffer */ \
	sprintf(*var, fmt,  __VA_ARGS__); \
} while(0)

#if defined(__cplusplus)
}
#endif

#endif // STDIO_EXT_H

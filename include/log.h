#ifndef DEBUG_H
#define DEBUG_H

#define LEVEL_PRINTALL 0
#define LEVEL_DEBUG 1
#define LEVEL_INFO 2
#define LEVEL_WARN 3
#define LEVEL_ERR 4

#ifdef LOG_LEVEL
#define __LOG_LEVEL LOG_LEVEL
#else
#ifdef DEBUG
#define __LOG_LEVEL LEVEL_DEBUG
#else
#define __LOG_LEVEL LEVEL_WARN
#endif /* DEBUG */
#endif /* LOG_LEVEL */

#define _print(level, level_name, fmt, ...) \
	do { \
		if (level >= __LOG_LEVEL) \
			fprintf(stderr, level_name "\t%s:%d:%s(): " fmt, __FILE__, \
				__LINE__, __func__, __VA_ARGS__); \
	} while (0)

#define print_debug(fmt, ...) _print(LEVEL_DEBUG, "DEBUG", fmt, __VA_ARGS__)
#define print_info(fmt, ...) _print(LEVEL_INFO, "INFO", fmt, __VA_ARGS__)
#define print_warning(fmt, ...) _print(LEVEL_WARN, "WARN", fmt, __VA_ARGS__)
#define print_error(fmt, ...) _print(LEVEL_ERR, "ERROR", fmt, __VA_ARGS__)

#endif /* DEBUG_H */

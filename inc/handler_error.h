/**
 * handler_error.h
 */
#ifndef _HANDLER_ERROR_
#define _HANDLER_ERROR_

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

#endif // _HANDLER_ERROR_

#ifndef STORAGE_H_
#define STORAGE_H_

#include <stddef.h>

int storage_init(void);
int storage_write(void *data, size_t len);
int storage_erase_oldest(void);

#endif /* STORAGE_H_ */
#include <linux/init.h>
struct wireway_dev_data
{
    unsigned int nr;
    char *dev_ptr;
};
int wireway_dev_init(void);
void wireway_dev_exit(void);

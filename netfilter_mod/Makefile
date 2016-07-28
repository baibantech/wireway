obj-m := wireway.o 
wireway-y = filter_func.o wireway_dev.o
    KERNELBUILD :=/lib/modules/$(shell uname -r)/build  
default:
	make -C $(KERNELBUILD) M=$(shell pwd) modules  
clean:
	rm -rf *.o *.ko *.mod.c .*.cmd *.markers *.order *.symvers .tmp_versions 

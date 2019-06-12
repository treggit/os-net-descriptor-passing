//
// Created by Anton Shelepov on 2019-05-31.
//

#ifndef OS_NET_FIFO_H
#define OS_NET_FIFO_H

#include <sys/stat.h>
#include <string>
#include <sys/types.h>
#include "../file_descriptor/file_descriptor.h"
#include <utility>


struct fifo {
    fifo();

    std::pair<int, int> get_in();
    std::pair<int, int> get_out();
    bool valid();

private:
    bool status = true;
    struct std::pair<file_descriptor, file_descriptor> in, out;

};


#endif //OS_NET_FIFO_H

/*
 * Nick Codignotto
 *
 * nick.codignotto@gmail.com / twitter: @nickcoding / blog: nickcoding.com
 * 
 */

#include <sstream>
#include "darkpool/engine/order_provider/order_provider.h"
#include "common/model/order_factory.h"

namespace darkpool {

FilenameStreamOrderProvider::FilenameStreamOrderProvider(std::string filename,int batch_size) : InputStreamOrderProvider(file_input_stream, batch_size) {

    file_input_stream.open(filename, std::ifstream::in);

    if (!file_input_stream.is_open()) {
        throw std::invalid_argument("Failed to open specified file");
    }

}

FilenameStreamOrderProvider::~FilenameStreamOrderProvider() {}


} // namespace darkpool
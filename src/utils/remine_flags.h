#ifndef __REMINE_FLAGS_H__
#define __REMINE_FLAGS_H__

#include "../utils/utils.h"
#include "../utils/config.h"
#include "../utils/parameters.h"

void parseReMineFlags(int argc, char* argv[])
{
    for (int i = 1; i < argc; ++ i) {
        //cout<<"POS TAG settings"<<argv[i]<<endl;
        if (!strcmp(argv[i], "--input_file")) {
            fromString(argv[++i], TEXT_TO_SEG_REMINE);
        } else if (!strcmp(argv[i], "--pos_file")) {
            fromString(argv[++ i], TEXT_TO_SEG_POS_TAGS_REMINE);
        } else if (!strcmp(argv[i], "--deps_file")) {
            fromString(argv[++ i], TEXT_TO_SEG_DEPS_REMINE);
        } else if (!strcmp(argv[i], "--ems_file")) {
            fromString(argv[++ i], TEST_EMS_REMINE);
        } else if (!strcmp(argv[i], "--mode")) {
            fromString(argv[++ i], MODE);
        } else if (!strcmp(argv[i], "--thread")) {
            fromString(argv[++ i], NTHREADS);
        } else if (!strcmp(argv[i], "--model")) {
            SEGMENTATION_MODEL_REMINE = argv[++ i];
        } else {
            fprintf(stderr, "[Warning] Unknown Parameter: %s\n", argv[i]);
        }
    }
}


#endif

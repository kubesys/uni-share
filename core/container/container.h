#ifndef __CONTAINER_H
#define __CONTAINER_H

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

#define PATH_LENGTH 512

#define RPC_CLIENT_PATH "/usr/local/unishare/bin/"
#define RPC_CLIENT_NAME "accelerator-client"

#define VMLU_CONFIG_PATH "/etc/uni-share/vm"

/**
 * Controller pid information file name
 */
#define PIDS_CONFIG_NAME "pids.config"

/**
 * Controller configuration file name
 */
#define CONTROLLER_CONFIG_NAME "vcuda.config"
#define PIDS_CONFIG_PATH (VMLU_CONFIG_PATH "/" PIDS_CONFIG_NAME)
#define CONTROLLER_CONFIG_PATH (VMLU_CONFIG_PATH "/" CONTROLLER_CONFIG_NAME)

int getStr(const char *buffer, const char *prefixstr, const char *endstr, char *poduid, char *podName);
int get_cgroupV2_data(const char *pid_cgroup, char *pod_uid, char *container_id,
                    size_t size, char *cont_name);
int get_cgroup_data(const char *pid_cgroup, char *pod_uid, char *container_id,
                    size_t size);
int checkCgroupVersion(); 
static int get_path_by_cgroup(int cgroupVersion);    
int read_controller_configuration();

typedef struct resource_data_t {
    char pod_uid[48];
    char container_name[FILENAME_MAX];
    uint64_t memory;
    int utilization;
} rc_data_t __attribute__((packed, aligned(8)));

#endif
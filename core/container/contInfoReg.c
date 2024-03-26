#include "hijack.h"
#include "container.h"


static char cgroupInfoPath[PATH_LENGTH];
char base_dir[PATH_LENGTH];
char pid_path[PATH_LENGTH];
char config_path[PATH_LENGTH];
rc_data_t config_data;


static const struct timespec g_cycle = {
    .tv_sec = 0,
    .tv_nsec = TIME_TICK * MILLISEC,
};

int getStr(const char *buffer, const char *prefixstr, const char *endstr, char *poduid, char *podName){
        size_t length = 0;
        char *start;
        char *end;
        char *nameEnd;

        start = strstr(buffer, prefixstr);
        end = strstr(buffer, endstr);
        if (start == NULL || end == NULL)
            return 0;
        length = end - (start + strlen(prefixstr));
        strncpy(poduid, start + strlen(prefixstr), length);
        poduid[length] = '\0';
        end += strlen(endstr);
        nameEnd = strchr(end, '/');
        length = nameEnd -end;
        strncpy(podName, end, length);
        podName[length] = '\0';

        return 1;
}


int get_cgroupV2_data(const char *pid_cgroup, char *pod_uid, char *container_id,
                    size_t size, char *cont_name) {
  
  char *prefix_pod = "/var/lib/kubelet/pods/";
  char *end_pod = "/containers/";
  

  int ret = 1;
  FILE *cgroup_fd = NULL;
  char buffer[4096];
  
  cgroup_fd = fopen(pid_cgroup, "r");
  if (unlikely(!cgroup_fd)) {
    LOGGER(4, "can't open %s, error %s", pid_cgroup, strerror(errno));
    goto DONE;
  }
  while (!feof(cgroup_fd)){
    buffer[0] = '\0';
    if (fgets(buffer, sizeof(buffer), cgroup_fd) != NULL)
    {
      
    }

    if (getStr(buffer, prefix_pod, end_pod, pod_uid, cont_name)) {
      break;
    }
  }


  ret = 0;
DONE:
  if (cgroup_fd) {
    fclose(cgroup_fd);
  }
  return ret;
}

int get_cgroup_data(const char *pid_cgroup, char *pod_uid, char *container_id,
                    size_t size) {
  int ret = 1;
  FILE *cgroup_fd = NULL;
  char *token = NULL, *last_ptr = NULL, *last_second = NULL;
  char *cgroup_ptr = NULL;
  char buffer[4096];
  int is_systemd = 0;
  char *prune_pos = NULL;

  cgroup_fd = fopen(pid_cgroup, "r");
  if (unlikely(!cgroup_fd)) {
    LOGGER(4, "can't open %s, error %s", pid_cgroup, strerror(errno));
    goto DONE;
  }

  /**
   * find memory cgroup name
   */
  while (!feof(cgroup_fd)) {
    buffer[0] = '\0';
    if (unlikely(!fgets(buffer, sizeof(buffer), cgroup_fd))) {
      LOGGER(4, "can't get line from %s", pid_cgroup);
      goto DONE;
    }

    buffer[strlen(buffer) - 1] = '\0';

    last_ptr = NULL;
    token = buffer;
    for (token = strtok_r(token, ":", &last_ptr); token;
         token = NULL, token = strtok_r(token, ":", &last_ptr)) {
      if (!strcmp(token, "memory")) {
        cgroup_ptr = strtok_r(NULL, ":", &last_ptr);
        break;
      }
    }

    if (cgroup_ptr) {
      break;
    }
  }

  if (!cgroup_ptr) {
    LOGGER(4, "can't find memory cgroup from %s", pid_cgroup);
    goto DONE;
  }

  /**
   * find container id
   */
  last_ptr = NULL;
  last_second = NULL;
  token = cgroup_ptr;
  while (*token) {
    if (*token == '/') {
      last_second = last_ptr;
      last_ptr = token;
    }
    ++token;
  }

  if (!last_ptr) {
    goto DONE;
  }

  strncpy(container_id, last_ptr + 1, size);
  container_id[size - 1] = '\0';

  /**
   * if cgroup is systemd, cgroup pattern should be like
   * /kubepods.slice/kubepods-besteffort.slice/kubepods-besteffort-pod27882189_b4d9_11e9_b287_ec0d9ae89a20.slice/docker-4aa615892ab2a014d52178bdf3da1c4a45c8ddfb5171dd6e39dc910f96693e14.scope
   * /kubepods.slice/kubepods-pod019c1fe8_0d92_4aa0_b61c_4df58bdde71c.slice/cri-containerd-9e073649debeec6d511391c9ec7627ee67ce3a3fb508b0fa0437a97f8e58ba98.scope
   */
  if ((prune_pos = strstr(container_id, ".scope"))) {
    is_systemd = 1;
    *prune_pos = '\0';
  }

  /**
   * find pod uid
   */
  *last_ptr = '\0';
  if (!last_second) {
    goto DONE;
  }

  strncpy(pod_uid, last_second, size);
  pod_uid[size - 1] = '\0';

  if (is_systemd && (prune_pos = strstr(pod_uid, ".slice"))) {
    *prune_pos = '\0';
  }

  /**
   * remove unnecessary chars from $container_id and $pod_uid
   */
  if (is_systemd) {
    /**
     * For this kind of cgroup path, we need to find the last appearance of
     * slash
     * /kubepods.slice/kubepods-pod019c1fe8_0d92_4aa0_b61c_4df58bdde71c.slice/cri-containerd-9e073649debeec6d511391c9ec7627ee67ce3a3fb508b0fa0437a97f8e58ba98.scope
     */
    prune_pos = NULL;
    token = container_id;
    while (*token) {
      if (*token == '-') {
        prune_pos = token;
      }
      ++token;
    }

    if (!prune_pos) {
      LOGGER(4, "no - prefix");
      goto DONE;
    }

    memmove(container_id, prune_pos + 1, strlen(container_id));

    prune_pos = strstr(pod_uid, "-pod");
    if (!prune_pos) {
      LOGGER(4, "no pod string");
      goto DONE;
    }
    prune_pos += strlen("-pod");
    memmove(pod_uid, prune_pos, strlen(prune_pos));
    pod_uid[strlen(prune_pos)] = '\0';
    prune_pos = pod_uid;
    while (*prune_pos) {
      if (*prune_pos == '_') {
        *prune_pos = '-';
      }
      ++prune_pos;
    }
  } else {
    memmove(pod_uid, pod_uid + strlen("/pod"), strlen(pod_uid));
  }

  ret = 0;
DONE:
  if (cgroup_fd) {
    fclose(cgroup_fd);
  }
  return ret;
}

int checkCgroupVersion(){
    FILE *fp;
    char buffer[4096];
    char *check;
    
    fp = popen("mount | grep cgroup", "r");
    if (unlikely(!fp)) {
        LOGGER(4, "popen error");
    }
    
    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        check = strstr(buffer, "/sys/fs/cgroup/memory");
        if (check != NULL){
            strcpy(cgroupInfoPath, "/proc/self/cgroup");
            pclose(fp);
            return 0;
        }
    }
    strcpy(cgroupInfoPath, "/proc/self/mountinfo");
    pclose(fp);

    return 1;
}

static int get_path_by_cgroup(int cgroupVersion) {
  int ret = 1;
  char pod_uid[4096], container_id[4096], cont_name[4096];

  cont_name[0] = '\0';

  if (cgroupVersion){
    if (unlikely(get_cgroupV2_data(cgroupInfoPath, pod_uid, container_id,
                               sizeof(container_id), cont_name))) {
    LOGGER(4, "can't find container name from %s", cgroupInfoPath);
    goto DONE;
    }
    snprintf(base_dir, sizeof(base_dir), "%s%s", VMLU_CONFIG_PATH, cont_name);
    snprintf(config_path, sizeof(config_path), "%s/%s", base_dir,
           CONTROLLER_CONFIG_NAME);
    snprintf(pid_path, sizeof(pid_path), "%s/%s", base_dir, PIDS_CONFIG_NAME);
  }
  else{
    if (unlikely(get_cgroup_data(cgroupInfoPath, pod_uid, container_id,
                               sizeof(container_id)))) {
    LOGGER(4, "can't find container id from %s", cgroupInfoPath);
    goto DONE;
    }
    snprintf(base_dir, sizeof(base_dir), "%s%s", VMLU_CONFIG_PATH, container_id);
    snprintf(config_path, sizeof(config_path), "%s/%s", base_dir,
           CONTROLLER_CONFIG_NAME);
    snprintf(pid_path, sizeof(pid_path), "%s/%s", base_dir, PIDS_CONFIG_NAME);
  }

  LOGGER(4, "config file: %s", config_path);
  LOGGER(4, "pid file: %s", pid_path);
  ret = 0;

DONE:
  return ret;
}

int read_controller_configuration() {
    int fd = 0;
    int rsize;
    int ret = 1;

    if (get_path_by_cgroup(checkCgroupVersion())) {
        LOGGER(FATAL, "can't get config file path");
    }
    fd = open(config_path, O_RDONLY);
    if (unlikely(fd == -1)) {
        LOGGER(4, "can't open %s, error %s", config_path, strerror(errno));
        goto DONE;
    }

    rsize = (int)read(fd, (void *)&config_data, sizeof(rc_data_t));
    if (unlikely(rsize != sizeof(config_data))) {
        LOGGER(4, "can't read %s, need %zu but got %d", CONTROLLER_CONFIG_PATH,
            sizeof(rc_data_t), rsize);
        goto DONE;
    }

    LOGGER(4, "pod_uid is %s", config_data.pod_uid);
    LOGGER(4, "container_name is %s", config_data.container_name);
    LOGGER(4, "memory limit is %ld", config_data.memory);
    LOGGER(4, "utilization limit is %d", config_data.utilization);

    ret = 0;

    DONE:
    if (likely(fd)) {
        close(fd);
    }

    return ret;
}


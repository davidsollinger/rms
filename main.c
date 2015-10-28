#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <stdbool.h>

#define BUF_SIZE 131072

#ifndef PATH_MAX
  #define PATH_MAX 4096
#endif

//fills file with random data
char writeOver(int fd, int filesize) {

  char buffer[BUF_SIZE];

  int dataLeft = filesize;
  int outSize = sizeof(buffer);

  while(dataLeft > 0) {
    size_t written = 0;
    //generate random data to write
    arc4random_buf(buffer, outSize);

    if (dataLeft < outSize) {
      //make sure we aren't writing more data than
      //the original file
      written = write(fd, buffer, dataLeft);
    } else
      written = write(fd, buffer, outSize);

    dataLeft -= written;

    //stop on failure to write
    if (written <= 0)
      break;

  }

  return errno;
}

int deleteFile(const char *filepath) {
  printf("Error pre is %s \n", strerror(errno));
  int fd = open(filepath, O_RDWR);
  if (fd < 0) {
    fprintf(stderr, "Failed opening file.\n");
    return 2;
  }

  struct stat st = {};
  lstat(filepath, &st);
  if (writeOver(fd, st.st_size)) {
    fprintf(stderr, "Error overwriting file.\n");
    return 3;
  }

  //close and delete file
  close(fd);
  unlink(filepath);
  return 0;
}

bool isDirectory(const char *filepath) {
  DIR *dir;
  if ((dir = opendir(filepath))) {
    closedir(dir);
    return true;
  }
  return false;
}

bool isDotDirectory(const char *filename) {
  return filename[0] == '.' && (strlen(filename) == 1 || (strlen(filename) == 2 && filename[1] == '.'));
}

const char *pathJoin(const char *dir, const char *file) {
  static char buffer[PATH_MAX + 1];
  buffer[0] = '\0';
  snprintf(buffer, PATH_MAX, "%s/%s", dir, file);
  return buffer;
}

int deleteFiles(const char *directory) {
  if (!directory || !strlen(directory))
    return 1;

  if (!isDirectory(directory))
    return deleteFile(directory);

  char dirpath[PATH_MAX] = {0};
  strcat(dirpath, directory);
  if (directory[strlen(directory) - 1] != '/')
    strcat(dirpath, "/");

  DIR *dir = opendir(dirpath);
  struct dirent *ent;
  while ((ent = readdir(dir))) {
    struct stat st;
    const char *filepath = pathJoin(dirpath, ent->d_name);
    lstat(filepath, &st);

    if (S_ISREG(st.st_mode))
      deleteFile(filepath);
    else if (S_ISDIR(st.st_mode) && !isDotDirectory(ent->d_name))
      deleteFiles(filepath);
  }
  closedir(dir);
  rmdir(dirpath);
  return 0;
}

int main (int argc, const char *argv[]) {

  if (argc < 2) {
    fprintf(stderr, "Missing filepath.\n");
    return 1;
  }

  if (!strcmp(argv[1], "-r") || !strcmp(argv[1], "-rf")) {
    if (argc < 3 || !argv[2]) {
      fprintf(stderr, "Missing filepath");
      return 1;
    }
    return deleteFiles(argv[2]);
  }
  else {
    return deleteFile(argv[1]);
  }
}
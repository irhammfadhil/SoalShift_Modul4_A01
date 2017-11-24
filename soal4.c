#define FUSE_USE_VERSION 28

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef linux
#define _XOPEN_SOURCE 500
#endif

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

static const char *dirpath = "/home/silvershadow/Downloads";

static int xmp_mknod();
static int xmp_getattr(const char *path, struct stat *stbuf)
{
    int res;
    char fpath[1000];
    sprintf(fpath,"%s%s",dirpath,path);

	res = lstat(path, stbuf);
	if (res == -1)
		return -errno;

	return 0;
}

static int xmp_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
    char fpath[1000];
    if(strcmp(path,"/") == 0)
    {
        path=dirpath;
        sprintf(fpath,"%s",path);
    }
    else sprintf(fpath,"%s%s",dirpath,path);
    int res = 0;    
    
	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;

	dp = opendir(path);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		if (filler(buf, de->d_name, &st, 0))
			break;
	}

	closedir(dp);
	return 0;
}

static int xmp_mkdir(const char *path, mode_t mode)
{
    int res;
    res = mkdir(path, mode);
    if(res==-1)
        return -errno;
    
    return 0;
}

static int xmp_unlink(const char *path)
{
    int res;
    char fpath[1000];
    sprintf(fpath,"%s%s",dirpath,path);

    res = unlink(fpath);
    if(res == -1)
        return -errno;

    return 0;
}

static int xmp_rename(const char *from, const char *to)
{
    int res;
    
    res = rename(from, to);
    if(res==-1)
        return -errno;

    return 0;
}

static int xmp_link(const char *from, const char *to)
{
    int res;
    
    res = link(from, to);
    if(res==-1)
        return -errno;

    return 0;
}

static int xmp_chmod(const char*path, mode_t mode)
{
    int res;
    char fpath[1000];
    sprintf(fpath,"%s%s",dirpath,path);

    res = chmod(fpath, mode);
    if(res==-1)
        return -errno;

    return 0;
}

static int xmp_open(const char *path, struct fuse_file_info *fi)
{
	int res;
	char fpath[1000];
        sprintf(fpath,"%s%s",dirpath,path);

	res = open(fpath, fi->flags);
	if (res == -1)
		return -errno;

	close(res);
	return 0;
}

static int xmp_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{   
    int fd;
	int res;
    char fpath[1000];
	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
    else sprintf(fpath, "%s%s",dirpath,path);

	(void) fi;
	fd = open(path, O_RDONLY);
	if (fd == -1)
		return -errno;

	res = pread(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int xmp_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	 int res = 0;
   int fd = 0 ;
	 char fpath[1000];
	 char alamat[1000];
	 char namafile[20];

	if(strcmp(path,"/") == 0)
	{
		path=dirpath;
		sprintf(fpath,"%s",path);
	}
	else {
	sprintf(namafile,"%s",path);
	sprintf(fpath, "%s%s",dirpath,path);
	}
	
	(void) fi;
	sprintf(alamat,"%s/simpanan/%s",dirpath,namafile);
	fd = open(alamat, O_WRONLY);
	if (fd == -1)
		return -errno;
	res = pwrite(fd, buf, size, offset);
	if (res == -1)
		res = -errno;

	close(fd);
	return res;
}

static int xmp_truncate(const char *path, off_t size)
{
	int res;
    char fpath[1000];
	char alamat[1000];
	char namafile[20];
    
    if(strcmp(path,"/") == 0)
    {
        path=dirpath;
        sprintf(fpath,"%s",path);
    } else 
        {
            sprintf(namafile,"%s",path);
            sprintf(fpath, "%s%s",dirpath,path);
        }

    sprintf(alamat,"%s/simpanan",dirpath);
    xmp_mkdir(alamat,0755); 
    
    sprintf(alamat,"%s/simpanan/%s",dirpath,namafile);
    xmp_mknod(alamat,0664);
    res = truncate(alamat, size);
    if (res == -1)
    return -errno;

	return 0;
}

static int xmp_mknod(const char *path, mode_t mode, dev_t rdev)
{
	int res;

	if (S_ISREG(mode)) 
    {
		res = open(path, O_CREAT | O_EXCL | O_WRONLY, mode);
		if (res >= 0)
		res = close(res);
	} else if (S_ISFIFO(mode))
		res = mkfifo(path, mode);
	else
		res = mknod(path, mode, rdev);
	if (res == -1)
		return -errno;

	return 0;
}

static struct fuse_operations xmp_oper = {
	.getattr	= xmp_getattr,
	.readdir	= xmp_readdir,
	.mkdir		= xmp_mkdir,
	.unlink		= xmp_unlink,
	.rename		= xmp_rename,
	.link		  = xmp_link,
	.chmod		= xmp_chmod,
	.open		  = xmp_open,
	.read		  = xmp_read,
	.write		= xmp_write,
	.truncate	= xmp_truncate,
  .mknod    = xmp_mknod,
};

int main(int argc, char *argv[])
{
	umask(0);
	return fuse_main(argc, argv, &xmp_oper, NULL);
}

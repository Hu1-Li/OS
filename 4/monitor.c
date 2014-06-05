#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include "fcntl.h"
#include <unistd.h>
#include <string.h>
#include <sys/vfs.h>
#include <mntent.h>
#include "dirent.h"
float IDLE, TOTAL;
GtkWidget *dialog;
int cpu_core;
enum {
	DEVICES_COLUMN,
	DIR_COLUMN,
	TYPE_COLUMN,
	TOTAL_COLUMN,
	FREE_COLUMN,
	AVAILABLE_COLUMN,
	USED_COLUMN
};
enum {
	PROCESS_COLUMN,
	STATUS_COLUMN,
	CPU_COLUMN,
	ID_COLUMN,
	MEMORY_COLUMN,
	WAIT_COLUMN,
	NICE_COLMN
};
typedef struct {
	char device[256];
	char mntpnt[256];
	char type[256];
	long blocks;    
	long bfree;
	long bused;
	long available_disk;
	int bused_percent;    
} DiskInfo;
void reboot(GtkWidget *window,gpointer data)
{
	system("reboot");
}
void shutdown(GtkWidget *window,gpointer data)
{
	system("shutdown now");
}
void about(GtkWidget *window, gpointer data)
{
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_hide(dialog);//DO NOT DESTYOR IT.JUST HIDE IT.
}
int get_disk_info(GtkListStore *store)
{
	gtk_list_store_clear(store);
	DiskInfo *disk_info;
	struct statfs fs_info;
	struct mntent *mnt_info;
	FILE *fh;
	float percent;     
	char disk_info_total[32];
	char disk_info_used[32];
	char disk_info_free[32];
	char disk_info_available[32];
	if((fh = setmntent( "/etc/mtab", "r" )) == NULL) {
		printf("Cannot open \'/etc/mtab\'!\n");
		return -1;
	}   
	while ((mnt_info = getmntent(fh)) != NULL) {
		if(statfs(mnt_info->mnt_dir, &fs_info) < 0) {
			continue;
		}
		if((disk_info = (DiskInfo *)malloc(sizeof(DiskInfo)))==NULL) {
			continue;    
		}    
		if(strcmp( mnt_info->mnt_type, "proc") &&
				strcmp(mnt_info->mnt_type, "devfs") &&
				strcmp(mnt_info->mnt_type, "vboxsf") &&
				strcmp(mnt_info->mnt_type, "devtmpfs") &&
				strcmp(mnt_info->mnt_type, "usbfs") &&
				strcmp(mnt_info->mnt_type, "sysfs") &&
				strcmp(mnt_info->mnt_type, "tmpfs") &&
				strcmp(mnt_info->mnt_type, "devpts")&&
				strcmp(mnt_info->mnt_type, "fusectl") && 
				strcmp(mnt_info->mnt_type, "debugfs") &&
				strcmp(mnt_info->mnt_type, "binfmt_misc")&&
				strcmp(mnt_info->mnt_type, "fuse.gvfs-fuse-daemon")&&
				strcmp(mnt_info->mnt_type, "securityfs")) {
			if(fs_info.f_blocks != 0) {
				percent = (((float)fs_info.f_blocks -(float)fs_info.f_bfree)
					       	* 100.0/ (float)fs_info.f_blocks);
			} else {
				percent = 0;
			}                
		} else {
			continue;    
		}
		strcpy(disk_info->type, mnt_info->mnt_type);        
		strcpy(disk_info->device, mnt_info->mnt_fsname);
		strcpy(disk_info->mntpnt, mnt_info->mnt_dir);
		long block_size = fs_info.f_bsize / 1024;
		disk_info->blocks = fs_info.f_blocks * block_size / 1024;
		disk_info->bfree = fs_info.f_bfree * block_size / 1024;
		disk_info->available_disk = fs_info.f_bavail * block_size / 1024;
		disk_info->bused = (fs_info.f_blocks - fs_info.f_bfree) * block_size /1024;
		disk_info->bused_percent = (int) percent;
		sprintf(disk_info_total, "%.2f GiB", 
				(float) disk_info->blocks / 1024.0);
		sprintf(disk_info_used, "%.2f GiB", 
				(float) disk_info->bused / 1024.0);
		sprintf(disk_info_free, "%.2f GiB", 
				(float) disk_info->bfree / 1024.0);
		sprintf(disk_info_available, "%.2f GiB", 
				(float) disk_info->available_disk / 1024.0);
		GtkTreeIter iter;
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter,
				DEVICES_COLUMN, disk_info->device,
				DIR_COLUMN, disk_info->mntpnt,
				TYPE_COLUMN, disk_info->type,
				TOTAL_COLUMN, disk_info_total,
				FREE_COLUMN, disk_info_free,
				AVAILABLE_COLUMN, disk_info_available,
				USED_COLUMN, disk_info_used,
				-1);
	};
	return TRUE;
}
int fresh_load(GtkWidget *label)
{
	int fd;
	char buffer[32];
	char *load_info[3];
	char str_load[64];
	fd = open("/proc/loadavg", O_RDONLY);
	read(fd, buffer, sizeof(buffer));
	close(fd);
	load_info[0] = strtok(buffer, " ");
	load_info[1] = strtok(NULL, " ");
	load_info[2] = strtok(NULL, " ");
	sprintf(str_load, "Load average for the last 1, 5, 15minutes :%s, %s, %s",
			load_info[0], load_info[1], load_info[2]);
	gtk_label_set_text((GtkLabel *)label, str_load);
	return TRUE;
}
int get_process_info(GtkListStore *store){
	gtk_list_store_clear(store);
	DIR *dir;
	struct dirent *entry;
	int fd1, j;
	FILE *fd2;
	char dir_buf[256];	
	char buffer[128];
	char *mem_info[26];
	char *delim = " ";
	GtkTreeIter iter;
	char rate_buffer[16];
	double mem;
	char mem_buffer[16];
	char dir_temp[256];
	char buffer_temp[32];
	char *cpu_time[6][2];
	char *process_time[20][2];
	char buffer_cpu_time[128];
	char buffer_process_time[128];
	double sum_cpu_time[2];
	double sum_process_time[2];
	double total_cpu_time, total_process_time;
	char process_cpu_rate[10];
	double temp_rate;
	int i;
	dir = opendir ("/proc");
	while ((entry = readdir(dir)) != NULL) {
		if ((entry->d_name[0] >= '0') && (entry->d_name[0] <= '9')) {
			sprintf(dir_buf, "/proc/%s/stat", entry->d_name);
			fd1 = open(dir_buf, O_RDONLY);
			read(fd1, buffer, sizeof(buffer));
			close(fd1);
			sprintf(dir_temp, "/proc/%s/wchan", entry->d_name);
			fd2 = fopen(dir_temp, "r");
			while(!feof(fd2)) {
				fgets(buffer_temp, 100, fd2);
			}
			fclose(fd2);
			mem_info[0] =  strtok(buffer, delim);
			int pid_info = atoi(mem_info[0]);
			for (j = 1; j < 26 ; j++) {
				mem_info[j] = strtok(NULL, delim);
			}
			char *process_name = strtok(strtok(mem_info[1], "("), ")");
			for(i = 0; i < 2; i++) {
				int k;
				int fd_1, fd_2;
				fd_1 = open("/proc/stat", O_RDONLY);
				read(fd_1, buffer_cpu_time, sizeof(buffer_cpu_time));
				close(fd_1);
				fd_2 = open(dir_buf, O_RDONLY);
				read(fd_2, buffer_process_time, sizeof(buffer_process_time));
				close(fd_2);
				cpu_time[0][i] = strtok(buffer_cpu_time, delim);
				sum_cpu_time[i] = 0;
				for(k = 1; k < 6; k++) {
					cpu_time[k][i] = strtok(NULL, delim);
					if((k >= 1) && (k <= 5)) {
						sum_cpu_time[i] += atof(cpu_time[k][i]);
					}
				}
				process_time[0][i] = strtok(buffer_process_time, delim);
				sum_process_time[i] = 0;
				for(k = 1; k < 20; k++) {
					process_time[k][i] = strtok(NULL, delim);
					if((k >= 13) && (k <= 16)) {
						sum_process_time[i] += atof(process_time[k][i]);
					}
				}
			}
			temp_rate = ((sum_process_time[0] * 100 / sum_cpu_time[0]) +
					(sum_process_time[1] * 100 / sum_cpu_time[0])) / 2;
			//事实上应该在时间上取样分析数据
			sprintf(process_cpu_rate, "%.2f", temp_rate);
			mem = atoi(mem_info[22]);
			mem = mem / (1024 * 1024);
			sprintf(mem_buffer, "%-.2f MiB",mem);
			gtk_list_store_append(store, &iter);
			gtk_list_store_set(store, &iter,
					PROCESS_COLUMN, process_name,
					STATUS_COLUMN, mem_info[2],
					CPU_COLUMN, process_cpu_rate,
					ID_COLUMN, pid_info,
					MEMORY_COLUMN, mem_buffer,
					WAIT_COLUMN, buffer_temp,
					NICE_COLMN, mem_info[18],
					-1);
		}
	}
	closedir (dir);
	return TRUE;
}
void get_system_info(GtkWidget *labelA,
		GtkWidget *labelB,
		GtkWidget *labelC,
		GtkWidget *labelE,
		GtkWidget *labelF)
{
	int fd_host, fd_release, fd_version, fd_mem, fd_cpu;
	char *info[4];
	char *info_host;
	char str_host[32];
	char buffer_host[30];
	char buffer_release[128];
	char *info_release;
	char *start_release;
	char str_release[30];
	char *start_codename;
	char buffer_version[30];
	char str_version[30];	
	char *info_version;
	char buffer_mem[30];
	char *info_mem;
	char str_mem[30];
	char buffer_cpu[1000];
	char *info_cpu[6];
	char *temp;
	char str_cpu[50];
	char *start_cpu;
	fd_host = open("/etc/hostname", O_RDONLY);
	read(fd_host, buffer_host, sizeof(buffer_host));
	close(fd_host);
	info_host = strtok(buffer_host, "\n");
	sprintf(str_host, "     %s", info_host);
	gtk_label_set_text((GtkLabel *)labelA, str_host);
	fd_release = open("/etc/lsb-release", O_RDONLY);
	read(fd_release, buffer_release, sizeof(buffer_release));
	close(fd_release);
	info[0] = strtok(buffer_release, "\n");	
	int i;
	for(i = 0; i < 3; i++) {
		info[i] = strtok(NULL, "\n");
	}
	start_release = strstr(info[0], "=");
	start_release++;
	start_codename = strstr(info[1], "=");
	start_codename++;
	sprintf(str_release, "			Release %s(%s)",
			start_release, start_codename);
	gtk_label_set_text((GtkLabel *)labelB, str_release);
	fd_version = open("/proc/sys/kernel/osrelease", O_RDONLY);
	read(fd_version, buffer_version, sizeof(buffer_version));
	close(fd_version);
	info_version = strtok(buffer_version, "\n");
	sprintf(str_version, "			Kernel Linux %s", info_version);
	gtk_label_set_text((GtkLabel *)labelC, str_version);
	fd_mem = open("/proc/meminfo", O_RDONLY);
	read(fd_mem, buffer_mem, sizeof(buffer_mem));
	close(fd_mem);
	info_mem = strtok(buffer_mem, " ");
	info_mem = strtok(NULL, " ");
	int j;
	float k;
	j = atoi(info_mem);
	k = j / 1024;
	sprintf(str_mem, "			Memory: %.1f MiB", k);
	gtk_label_set_text((GtkLabel *)labelE, str_mem);
	fd_cpu = open("/proc/cpuinfo", O_RDONLY);
	read(fd_cpu, buffer_cpu, sizeof(buffer_cpu));
	close(fd_cpu);
	info_cpu[0] = strtok(buffer_cpu, "\n");
	int m;
	for(m = 0; m < 5; m++) {
		info_cpu[m + 1] = strtok(NULL, "\n");
	}
	start_cpu = strstr(info_cpu[4], ":");
	start_cpu++;
	start_cpu++;
	sprintf(str_cpu, "			Processor: %s x %d", start_cpu, cpu_core);
	gtk_label_set_text((GtkLabel *)labelF, str_cpu);
}
int get_cpu_rate_info(GtkWidget *progressbar)
{
	int fd;
	char buffer[128];
	float cpu_rate_info;
	char *buffer_cpu_rate[8];
	char text_cpu[20];
	float total = 0;
	fd = open("/proc/stat", O_RDONLY);
	read(fd, buffer, sizeof(buffer));
	close(fd);
	buffer_cpu_rate[0] = strtok(buffer, " ");
	int i;
	for(i = 1; i < 5; i++) {
		buffer_cpu_rate[i] = strtok(NULL, " ");
		total += atof(buffer_cpu_rate[i]);
	}
	cpu_rate_info = (total - TOTAL - atof(buffer_cpu_rate[4]) +
			IDLE) * 100 / (total - TOTAL);
	TOTAL = total;
	IDLE = atof(buffer_cpu_rate[4]);
	sprintf(text_cpu, "%.2f percents", cpu_rate_info);
	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progressbar), cpu_rate_info / 100);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progressbar), text_cpu);
	return TRUE;
}
int get_swap_rate_info(GtkWidget *progressbar)
{
	int fd;
	char buffer[128];
	int i;
	float swap_rate_info;
	char text_swap[32];
	char *buffer_swap_info[8];
	fd = open("/proc/swaps", O_RDONLY);
	read(fd, buffer, sizeof(buffer));
	close(fd);
	buffer_swap_info[0] = strtok(buffer, "\n");
	buffer_swap_info[1] = strtok(NULL, "\n");
	buffer_swap_info[2] = strtok(buffer_swap_info[1], " 	");
	for(i = 3; i < 8; i++) {
		buffer_swap_info[i] = strtok(NULL, " 	");
	}
	if(0 == atof(buffer_swap_info[5])) {
		swap_rate_info = 0.0;
	} else {
		swap_rate_info = atof(buffer_swap_info[5]) * 100.0 / atof(buffer_swap_info[4]);
	}
	sprintf(text_swap, "%.2f percents", swap_rate_info);
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(progressbar), swap_rate_info / 100);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progressbar), text_swap);
	return TRUE;
}
int get_mem_rate_info(GtkWidget *progressbar)
{
	int fd;
	char buffer[128];
	float buffer_mem_info[8];
	float mem_rate_info;
	char text_mem[32];
	fd =open("/proc/meminfo", O_RDONLY);
	read(fd, buffer, sizeof(buffer));
	close(fd);
	buffer_mem_info[0] = atof(strtok(buffer, " "));
	int i;
	for(i = 1; i < 8; i++) {
		buffer_mem_info[i] = atof(strtok(NULL, " "));
	}
	mem_rate_info = (buffer_mem_info[1] - buffer_mem_info[3] -
			buffer_mem_info[5] - buffer_mem_info[7]) * 100 / (buffer_mem_info[1]);
	sprintf(text_mem, "%.2f percents", mem_rate_info);
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(progressbar), mem_rate_info / 100);
	gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progressbar), text_mem);
	return TRUE;
}
int fresh_uptime(GtkWidget *label)
{
	FILE *fp_uptime;
	char buffer_uptime[32];//读文件
	char *info_uptime[3];//分割
	char str_uptime[128];//输出缓冲
	fp_uptime = fopen("/proc/uptime", "r");
	while(!feof(fp_uptime)) {
		fgets(buffer_uptime, 100, fp_uptime);
	}
	fclose(fp_uptime);
	info_uptime[0] = strtok(buffer_uptime, " ");
	info_uptime[1] = strtok(NULL, " ");
	int uptime_temp = (int)(atof(info_uptime[1]) / cpu_core);
	sprintf(str_uptime, "\t\t\tSystem runs for %d hours %d minutes %d seconds",
			uptime_temp/ 3600, uptime_temp % 3600 / 60, uptime_temp % 60);
	gtk_label_set_text((GtkLabel *)label, str_uptime);
	return TRUE;
}
int get_cpu_core()
{
	int fd;
	int i;
	char *buffer_cpu_info[9];
	char buffer[2048];
	char *pch;
	fd = open("/proc/stat", O_RDONLY);
	read(fd, buffer, sizeof(buffer));
	close(fd);
	buffer_cpu_info[0] = strtok(buffer, "\n");
	for(i = 1; i < 9; i++) {
		buffer_cpu_info[i] = strtok(NULL, "\n");	 
	}
	//8核??
	for(i = 0; i < 9; i++) {
		pch = strchr(buffer_cpu_info[i], 'c');
		if((pch != NULL) && (strchr(++pch, 'p') != NULL)
				&& (strchr(++pch, 'u') != NULL)) {
			cpu_core++;
		}
	}
	return (--cpu_core);
}
int main(int argc, char *argv[])
{
	GtkBuilder *builder;
	GtkWidget *notebook;
	GtkWidget *window;
	GtkWidget *quit_menu, *reboot_menu, *shutdown_menu, *about_menu;
	GtkWidget *label1, *label2, *label3, *label4, *label5, *label7, *label8;
	GtkWidget *label9, *label11,*label12, *label14, *label15;
	GtkListStore *store_disk, *store_process;
	GtkWidget *progressbar_cpu, *progressbar_mem, *progressbar_swap;
	GtkWidget *fresh_button;
	gtk_init(&argc, &argv);
	builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, "monitor.glade", NULL);
	window = GTK_WIDGET(gtk_builder_get_object(builder, "window"));
	quit_menu = GTK_WIDGET(gtk_builder_get_object(builder, "imagemenuitem5"));
	shutdown_menu = GTK_WIDGET(gtk_builder_get_object(builder, "imagemenuitem3"));
	reboot_menu = GTK_WIDGET(gtk_builder_get_object(builder, "imagemenuitem4"));
	about_menu = GTK_WIDGET(gtk_builder_get_object(builder, "imagemenuitem10"));
	notebook = GTK_WIDGET(gtk_builder_get_object(builder, "notebook"));
	label1 = GTK_WIDGET(gtk_builder_get_object(builder, "label1"));
	label2 = GTK_WIDGET(gtk_builder_get_object(builder, "label2"));
	label3 = GTK_WIDGET(gtk_builder_get_object(builder, "label3"));
	label4 = GTK_WIDGET(gtk_builder_get_object(builder, "label4"));
	label5 = GTK_WIDGET(gtk_builder_get_object(builder, "label5"));
	label7 = GTK_WIDGET(gtk_builder_get_object(builder, "label7"));
	label8 = GTK_WIDGET(gtk_builder_get_object(builder, "label8"));
	label11 = GTK_WIDGET(gtk_builder_get_object(builder, "label11"));
	label12 = GTK_WIDGET(gtk_builder_get_object(builder, "label12"));
	label14 = GTK_WIDGET(gtk_builder_get_object(builder, "label14"));
	label15 = GTK_WIDGET(gtk_builder_get_object(builder, "label15"));
	dialog = GTK_WIDGET(gtk_builder_get_object(builder, "aboutdialog"));
	progressbar_cpu = GTK_WIDGET(gtk_builder_get_object(builder, "progressbar_cpu"));
	progressbar_mem = GTK_WIDGET(gtk_builder_get_object(builder, "progressbar_mem"));
	progressbar_swap = GTK_WIDGET(gtk_builder_get_object(builder, "progressbar_swap"));
	get_cpu_core();
	get_system_info(label5, label7, label8, label11, label12);
	store_disk = GTK_LIST_STORE(gtk_builder_get_object(builder, "store_disk"));
	store_process = GTK_LIST_STORE(gtk_builder_get_object(builder, "store_process"));
	g_timeout_add(1000, (GSourceFunc)fresh_uptime, (gpointer)label14);
	g_timeout_add(1000, (GSourceFunc)fresh_load, (gpointer)label15);
	g_timeout_add(1000, (GSourceFunc)get_cpu_rate_info, (gpointer)progressbar_cpu);
	g_timeout_add(1000, (GSourceFunc)get_mem_rate_info, (gpointer)progressbar_mem);
	g_timeout_add(1000, (GSourceFunc)get_swap_rate_info, (gpointer)progressbar_swap);
	get_disk_info(store_disk);
	get_process_info(store_process);
	g_timeout_add(10000, (GSourceFunc)get_disk_info, (gpointer)store_disk);
	g_timeout_add(10000, (GSourceFunc)get_process_info, (gpointer)store_process);
	g_signal_connect(GTK_WINDOW(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(quit_menu, "activate", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(reboot_menu, "activate", G_CALLBACK(reboot), NULL);
	g_signal_connect(shutdown_menu, "activate", G_CALLBACK(shutdown), NULL);
	g_signal_connect(about_menu, "activate", G_CALLBACK(about), NULL);
	gtk_widget_show(window);
	g_object_unref(builder);
	gtk_main();
	return 0;
}

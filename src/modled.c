#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

#include "modled.h"

static void print_usage(const char *progname)
{
	printf("usage: %s [-d] [-h] [-i] [-b LEVEL] [-t EVENT] [LED]\n", progname);
}

static void print_warning(char *progname, uid_t euid)
{
	if (euid)
		printf("Warning: Running %s as non-root (EUID: %d). Expect issues.\n", progname, euid);
}

static void print_led_list(void)
{
	DIR *dir = opendir(LED_ROOT_DIR);

	if (!dir) {
		perror(LED_ROOT_DIR);
		exit(EXIT_FAILURE);
	}

	struct dirent *ent;

	puts("LED list:");

	while ((ent = readdir(dir))) {
		if (ent->d_name[0] != '.' && ent->d_type == DT_LNK)
			printf("\t%s\n", ent->d_name);
	}

	closedir(dir);
}

static void getargs(int argc, char **argv, struct led_args *args)
{
	int opt;

	while ((opt = getopt(argc, argv, ":dhib:t:")) != -1) switch (opt) {
	case 'd':
		print_led_list();
		exit(EXIT_SUCCESS);
		break;
	case 'h':
		print_usage(argv[0]);
		exit(EXIT_SUCCESS);
		break;
	case 'i':
		args->show_info = 1;
		break;
	case 'b':
		args->brightness = optarg;
		break;
	case 't':
		args->trigger = optarg;
		break;
	}

	if (optind != argc)
		args->LED = argv[optind];
}

static size_t read_to_buffer(const char *filename, size_t bsize, char *buffer)
{
	FILE *fp = fopen(filename, "r");

	if (!fp) {
		perror(filename);
		return 0;
	}

	size_t bytes = fread(buffer, sizeof(char), bsize, fp);
	fclose(fp);

	return bytes;
}

static size_t read_in(const char *dir, const char *file, size_t bsize, char *buffer)
{
	char fname[MAX_BUFFER];
	snprintf(fname, MAX_BUFFER, "%s%s", dir, file);

	size_t bytes = read_to_buffer(fname, bsize - 1, buffer);
	buffer[bytes] = '\0';
	buffer[strcspn(buffer, "\n")] = '\0';

	return bytes;
}

static void write_to_file(const char *filename, const char *value)
{
	FILE *fp = fopen(filename, "w");

	if (!fp)
		return perror(filename);

	fputs(value, fp);

	fflush(fp);
	fclose(fp);
}

static void write_out(const char *dir, const char *file, const char *value)
{
	char buffer[MAX_BUFFER];

	snprintf(buffer, MAX_BUFFER, "%s%s", dir, file);

	write_to_file(buffer, value);
}

static void display_brightness(const char *dir)
{
	char level[LARGE_BUFFER];
	char max[LARGE_BUFFER];

	if (
		read_in(dir, LED_BRIGHTNESS, LARGE_BUFFER, level) &&
		read_in(dir, LED_MAX_BRIGHTNESS, LARGE_BUFFER, max)
	)
		printf("BRIGHTNESS/MAX_BRIGHTNESS:\n\t%s/%s\n", level, max);
}

static void display_triggers(const char *dir)
{
	char list[LARGE_BUFFER];

	if (read_in(dir, LED_TRIGGER, LARGE_BUFFER, list))
		printf("TRIGGERS [current]:\n\t%s\n", list);
}

static void display_info(const char *dir)
{
	printf("LED:\n\t%s\n", dir);
	display_brightness(dir);
	display_triggers(dir);
}

static void validate_led(const char *led)
{
	if (!led) {
		fprintf(stderr, "Please specify an LED.\n");
		exit(EXIT_FAILURE);
	}

	if (strchr(led, '/')) {
		fprintf(stderr, "Bad LED: %s\n", led);
		exit(EXIT_FAILURE);
	}
}

static void validate_directory(const char *path)
{
	struct stat st;

	if (stat(path, &st) < 0 || !S_ISDIR(st.st_mode)) {
		fprintf(stderr, "Could not access LED: %s -- ", path);
		perror(NULL);
		exit(EXIT_FAILURE);
	}
}

static void modled(struct led_args *args, char **argv)
{
	validate_led(args->LED);

	char dir[MAX_BUFFER];

	snprintf(dir, sizeof(dir), LED_ROOT_DIR "%s/", args->LED);
	validate_directory(dir);

	if (args->brightness || args->trigger)
		print_warning(argv[0], geteuid());
	if (args->brightness)
		write_out(dir, LED_BRIGHTNESS, args->brightness);
	if (args->trigger)
		write_out(dir, LED_TRIGGER, args->trigger);
	if (args->show_info)
		display_info(dir);
}

int main(int argc, char **argv)
{
	struct led_args args = DEFAULT_LED_ARGS;

	getargs(argc, argv, &args);
	modled(&args, argv);

	return EXIT_SUCCESS;
}

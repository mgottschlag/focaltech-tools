
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define CMD_SET_SCALING_11 0xe6
#define CMD_SET_RESOLUTION 0xe8
#define CMD_STATUS_REQUEST 0xe9
#define CMD_ENABLE_DATA_REPORTING 0xf4
#define CMD_DISABLE_DATA_REPORTING 0xf5
#define CMD_RESET 0xff

void panic(const char *message)
{
	printf("Error: %s", message);
	exit(EXIT_FAILURE);
}

static int write_byte(int fd, unsigned char value)
{
	int ret;
	do {
		ret = write(fd, &value, 1);
		if (ret < 0) {
			return ret;
		}
	} while (ret == 0);
	return 0;
}

static int read_byte(int fd)
{
	int ret;
	unsigned char value;
	do {
		ret = read(fd, &value, 1);
		if (ret < 0) {
			return ret;
		}
	} while (ret == 0);
	return value;
}


int serio_reset(int file)
{
	int received;
	int device_id;

	if (write_byte(file, CMD_RESET) < 0) {
		return -1;
	}
	/* reset might not be acked properly, the mouse sends 0xaa after its
	 * self-test though. */
	do {
		received = read_byte(file);
		if (received < 0) {
			return -1;
		}
	} while (received != 0xaa);
	/* also read the device ID */
	device_id = read_byte(file);
	if (device_id < 0) {
		return -1;
	}
	printf("reset, device id = %02x\n", device_id); return 0; }

void send_command(int file, int command)
{
	if (write_byte(file, command) < 0) {
		panic("write_byte failed.");
	}
	if (read_byte(file) != 0xfa) {
		panic("ack missing.");
	}
}

void send_status_request(int file)
{
	int i;
	int received;

	if (write_byte(file, CMD_STATUS_REQUEST) < 0) {
		panic("write_byte failed.");
	}
	if (read_byte(file) != 0xfa) {
		panic("ack missing.");
	}
	printf("status:");
	for (i = 0; i < 3; i++) {
		received = read_byte(file);
		if (received < 0) {
			panic("read_byte failed.");
		}
		printf(" %02x", received);
	}
	printf("\n");
}

int dump_packets(int file, int length)
{
	int i;
	int received;

	while (1) {
		printf("packet:");
		for (i = 0; i < length; i++) {
			received = read_byte(file);
			if (received < 0) {
				return received;
			}
			printf(" %02x", received);
		}
		printf("\n");
	}
	return 0;
}



int main(int argc, char **argv)
{
	int i, j;
	(void)i;
	(void)j;

	if (argc != 2) {
		printf("Error: Invalid parameters.\n");
		printf("Usage: %s <path-to-serio_raw-device>\n", argv[0]);
		printf("For hints on how to enable serio_raw, see "
				"http://people.canonical.com/~sforshee/touchpad/mouse-to-serio.sh.\n");
		return -1;
	}

	int file = open(argv[1], O_RDWR);
	if (file == -1) {
		printf("Could not open %s. Check that the program is run "
				"with the required privileges and that the file exists.\n",
				argv[1]);
		perror("open");
		return -1;
	}

	if (serio_reset(file) < 0) {
		printf("reset failed.");
		return -1;
	}

#define COMPAT_PROTOCOL 0

#if COMPAT_PROTOCOL
	send_command(file, CMD_ENABLE_DATA_REPORTING);

	if (dump_packets(file, 3) < 0) {
		printf("dump_packets failed");
		return -1;
	}
#else
	send_command(file, CMD_DISABLE_DATA_REPORTING);
	send_command(file, CMD_SET_SCALING_11);
	send_command(file, CMD_SET_RESOLUTION);
	send_command(file, 0);
	send_command(file, CMD_SET_RESOLUTION);
	send_command(file, 0);
	send_command(file, CMD_SET_RESOLUTION);
	send_command(file, 0);
	send_command(file, CMD_SET_RESOLUTION);
	send_command(file, 0);
	send_status_request(file);
	send_command(file, CMD_SET_SCALING_11);
	send_command(file, CMD_SET_RESOLUTION);
	send_command(file, 0);
	send_command(file, CMD_SET_RESOLUTION);
	send_command(file, 0);
	send_command(file, CMD_SET_RESOLUTION);
	send_command(file, 0);
	send_command(file, CMD_SET_RESOLUTION);
	send_command(file, 1);
	send_status_request(file);
	send_command(file, CMD_SET_SCALING_11);
	send_command(file, CMD_SET_RESOLUTION);
	send_command(file, 0);
	send_command(file, CMD_SET_RESOLUTION);
	send_command(file, 0);
	send_command(file, CMD_SET_RESOLUTION);
	send_command(file, 0);
	send_command(file, CMD_SET_RESOLUTION);
	send_command(file, 2);
	send_status_request(file);
	send_command(file, CMD_SET_SCALING_11);
	send_command(file, CMD_SET_RESOLUTION);
	send_command(file, 0);
	send_command(file, CMD_SET_RESOLUTION);
	send_command(file, 0);
	send_command(file, CMD_SET_RESOLUTION);
	send_command(file, 0);
	send_command(file, CMD_SET_RESOLUTION);
	send_command(file, 5);
	send_status_request(file);
	send_command(file, CMD_SET_SCALING_11);
	send_command(file, CMD_SET_RESOLUTION);
	send_command(file, 0);
	send_command(file, CMD_SET_RESOLUTION);
	send_command(file, 0);
	send_command(file, CMD_SET_RESOLUTION);
	send_command(file, 0);
	send_command(file, CMD_SET_RESOLUTION);
	send_command(file, 6);
	send_status_request(file);
	send_command(file, 0xf8);
	send_command(file, 0);
	send_command(file, 0xf8);
	send_command(file, 0);
	send_command(file, 0xf8);
	send_command(file, 0);
	send_command(file, 0xf8);
	send_command(file, 1);
	send_command(file, CMD_SET_SCALING_11);
	send_command(file, CMD_ENABLE_DATA_REPORTING);

	if (dump_packets(file, 6) < 0) {
		printf("dump_packets failed");
		return -1;
	}
#endif

	return 0;
}


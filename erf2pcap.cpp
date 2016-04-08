#include <libtrace.h>
#include <stdio.h>
#include <unistd.h>

#include <algorithm>
#include <iostream>

using namespace std;

char* get_cmd_option(char **begin, char **end, const string &option) {
	char **iter = find(begin, end, option);
	if (iter != end && ++iter != end) {
		return *iter;
	}

	return 0;
}

static void cleanup(libtrace_t *in, libtrace_out_t *out, libtrace_packet_t *packet) {
	if (in) {
		trace_destroy(in);
	}

	if (out) {
		trace_destroy_output(out);
	}

	if (packet) {
		trace_destroy_packet(packet);
	}
}

int main(int argc, char *argv[]){
	libtrace_packet_t *packet = NULL;
	libtrace_t *in = NULL;
	libtrace_out_t *out = NULL;

	//parse arguements
	if (argc < 4) {
		cerr << "Usage: " << argv[0] << "start_time_hour end_time_hour out_file in_files..." << endl;
		return 1;
	}

	int start_time = atoi(argv[1]), end_time = atoi(argv[2]);

	//create packet
	packet = trace_create_packet();
	if (packet == NULL) {
		cerr << "Unable to create libtrace packet" << endl;
		cleanup(in, out, packet);
		return 1;
	}

	//create output trace
	out = trace_create_output(argv[3]);
	if (trace_is_err_output(out)) {
		trace_perror_output(out, "Opening output trace file");
		cleanup(in, out, packet);
		return 1;
	}

	if (trace_start_output(out) == -1) {
		trace_perror_output(out, "Starting output trace");
		cleanup(in, out, packet);
		return 1;
	}

	for (int i=4; i<argc; i++) {
		//open trace
		in = trace_create(argv[i]);
		if (trace_is_err(in)) {
			trace_perror(in, "Opening input trace file");
			cleanup(in, out, packet);
			return 1;
		}

		if (trace_start(in) == -1) {
			trace_perror(in, "Starting input trace");
			cleanup(in, out, packet);
			return 1;
		}


		//loop through packets
		int packet_count = 0;
		struct timeval ts;
		while (trace_read_packet(in, packet) > 0) {
			ts = trace_get_timeval(packet);
			int day_seconds = ts.tv_sec % 86400;

			if (day_seconds >= (3600 * start_time) && day_seconds <= (3600 * end_time)) {
				if (trace_write_packet(out, packet) == -1) {
					trace_perror_output(out, "Writing packet");
					cleanup(in, out, packet);
					return 1;
				}

				packet_count++;
			}
		}
		cout << "file:" << argv[i] << " packet_count:" << packet_count << endl;

		trace_destroy(in);
	}

	cleanup(NULL, out, packet);
	return 0;
}

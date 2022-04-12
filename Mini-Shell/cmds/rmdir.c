#include <stdio.h>
#include <unistd.h>

int main(int argc, char ** argv){
	
	// If no arguments are specified, print "Atleast 1 argument required"
	if (argc == 1){
		fprintf(stderr, "MY_SHELL: Atleast 1 argument required\n");
		return 1;
	}
	int i = 1;
	for (i = 1 ; i < argc ; i++){
		// Try deleting the directory
		if ( rmdir(argv[i]) < 0 ){
			fprintf(stderr, "MY_SHELL: %s - ", argv[i]);
			perror("");
			continue;
		}
	}
	return 0;
}

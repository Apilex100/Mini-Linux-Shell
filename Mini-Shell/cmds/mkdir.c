#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h> 

int main(int argc, char ** argv){
	
	// If no arguments are specified, print "Atleast 1 argument required"
	if (argc == 1){
		fprintf(stderr, "MY_SHELL: Atleast 1 argument required\n");
		return 1;
	}

	int i = 1;
	for (i = 1 ; i < argc ; i++){
		// Try creating the directory
		if ( mkdir(argv[i], 0755) < 0 ){
			fprintf(stderr, "MY_SHELL: %s - ", argv[i]);
			perror("Oops, somehting went wrong T-T . Please try again");
			continue;
		}
	}
	return 0;
}

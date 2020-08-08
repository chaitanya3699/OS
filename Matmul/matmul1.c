//header files
#include <stdlib.h>		/* for exit, atoi */
#include <stdio.h>		/* for fprintf */
#include <errno.h>		/* for error code eg. E2BIG */
#include <getopt.h>		/* for getopt */
#include <assert.h>		/* for assert */
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/stat.h>
#define NUM_THREADS 4
/*
 * Forward declarations
 */

void usage (int argc, char *argv[]);
void input_matrix (int *mat, int nrows, int ncols);
void output_matrix (int *mat, int nrows, int ncols);
void init_matrix (int *mat, int nrows, int ncols);
unsigned long long single_thread_mm ();
void matmul ();
unsigned long long multi_thread_mm ();
unsigned long long multi_process_mm ();

int *A, *B, *C;
int crows, ccols;
int arows, acols, brows, bcols;
char interactive = 0;
int p = 0;

//main function
int
main (int argc, char *argv[])
{
  int c;

  /* Loop through each option (and its's arguments) and populate variables */
  while (1)
    {
      int this_option_optind = optind ? optind : 1;
      int option_index = 0;
      static struct option long_options[] = {
	{"help", no_argument, 0, 'h'},
	{"ar", required_argument, 0, '1'},
	{"ac", required_argument, 0, '2'},
	{"br", required_argument, 0, '3'},
	{"bc", required_argument, 0, '4'},
	{"interactive", no_argument, 0, '5'},
	{0, 0, 0, 0}
      };

      c = getopt_long (argc, argv, "h1:2:3:4:5", long_options, &option_index);
      if (c == -1)
	break;

      switch (c)
	{
	case 0:
	  fprintf (stdout, "option %s", long_options[option_index].name);
	  fflush (stdout);
	  if (optarg)
	    fprintf (stdout, " with arg %s", optarg);
	  fflush (stdout);
	  fprintf (stdout, "\n");
	  fflush (stdout);
	  break;

	case '1':
	  arows = atoi (optarg);
	  break;

	case '2':
	  acols = atoi (optarg);
	  break;

	case '3':
	  brows = atoi (optarg);
	  break;

	case '4':
	  bcols = atoi (optarg);
	  break;

	case '5':
	  interactive = 1;
	  break;

	case 'h':
	case '?':
	  usage (argc, argv);

	default:
	  fprintf (stdout, "?? getopt returned character code 0%o ??\n", c);
	  fflush (stdout);
	  usage (argc, argv);
	}
    }

  if (optind != argc)
    {
      fprintf (stderr, "Unexpected arguments\n");
      usage (argc, argv);
    }
  //invalid input
  if (acols != brows)
    {
      exit (EXIT_FAILURE);
    }
  //initializing crows,ccols
  crows = arows;
  ccols = bcols;

  unsigned long long time_single, time_multi_process, time_multi_thread;
  //computing times taken 
  time_single = single_thread_mm ();
  time_multi_process = multi_process_mm ();
  time_multi_thread = multi_thread_mm ();

  fprintf (stdout, "Time taken for single threaded: %llu us\n", time_single);
  fflush (stdout);
  fprintf (stdout, "Time taken for multi process: %llu us\n",
	   time_multi_process);
  fflush (stdout);
  fprintf (stdout, "Time taken for multi threaded: %llu us\n",
	   time_multi_thread);
  fflush (stdout);
  fprintf (stdout, "Speedup for multi process : %4.2f x\n",
	   (double) time_single / time_multi_process);
  fflush (stdout);
  fprintf (stdout, "Speedup for multi threaded : %4.2f x\n",
	   (double) time_single / time_multi_thread);
  fflush (stdout);

  exit (EXIT_SUCCESS);
}

/*
 * Show usage of the program
 */
void
usage (int argc, char *argv[])
{
  fprintf (stderr, "Usage:\n");
  fprintf (stderr, "%s --ar <rows_in_A>  --ac <cols_in_A>"
	   " --br <rows_in_B>  --bc <cols_in_B>" " [--interactive]", argv[0]);
  exit (EXIT_FAILURE);
}

/*
 * Input a given 2D matrix
 */
void
input_matrix (int *mat, int rows, int cols)
{
  for (int i = 0; i < rows; i++)
    {
      for (int j = 0; j < cols; j++)
	{
	  fscanf (stdin, "%d", mat + (i * cols + j));
	}
    }
}

/*
 * Output a given 2D matrix
 */
void
output_matrix (int *mat, int rows, int cols)
{
  for (int i = 0; i < rows; i++)
    {
      for (int j = 0; j < cols; j++)
	{
	  fprintf (stdout, "%d ", *((mat + i * cols) + j));
	  fflush (stdout);
	}
      fprintf (stdout, "\n");
      fflush (stdout);
    }
}

/*
 * Input a given 2D matrix by random values
 */
void
init_matrix (int *mat, int rows, int cols)
{
  for (int i = 0; i < rows; i++)
    {
      for (int j = 0; j < cols; j++)
	{
	  *((mat + i * cols) + j) = rand ();
	}
    }
}

//function which returns the time taken for a single threaded process
unsigned long long
single_thread_mm ()
{
  //allocating memory
  A = (int *) malloc (arows * acols * sizeof (int));
  B = (int *) malloc (brows * bcols * sizeof (int));
  C = (int *) malloc (crows * ccols * sizeof (int));

  struct timeval start, end;	//to compute the time 

  if (interactive == 1)
    {				//interactive
      printf ("Enter A:\n");
      fflush (stdout);
      input_matrix (A, arows, acols);
      printf ("Enter B:\n");
      fflush (stdout);
      input_matrix (B, brows, bcols);
    }
  else if (interactive == 0)
    {				//non interactive
      init_matrix (A, arows, acols);
      init_matrix (B, brows, bcols);
    }

  gettimeofday (&start, NULL);
  //computing matrix multiplication
  for (int i = 0; i < crows; i++)
    {
      for (int j = 0; j < ccols; j++)
	{
	  *((C + i * ccols) + j) = 0;
	  for (int k = 0; k < acols; k++)
	    {
	      *((C + i * ccols) + j) +=
		*((A + i * acols) + k) * *((B + k * bcols) + j);
	    }
	}
    }
  gettimeofday (&end, NULL);

  //printing the output
  if (interactive == 1)
    {
      printf ("Result:\n");
      fflush (stdout);
      output_matrix (C, crows, ccols);
    }

  unsigned long long time = (end.tv_sec - start.tv_sec) * 1e6;
  time = (time + (end.tv_usec - start.tv_usec));
  //freeing the pointers
  free (A);
  free (B);
  free (C);
  return time;
}

//function to compute multiplication of matrices for multiple threads
void
matmul ()
{
  int q = p++;
  //each threads computes 1/4 th part of resultant matrix
  for (int i = (q * arows) / 4; i < ((q + 1) * arows) / 4; i++)
    {
      for (int j = 0; j < ccols; j++)
	{
	  *((C + i * ccols) + j) = 0;
	  for (int k = 0; k < acols; k++)
	    {
	      *((C + i * ccols) + j) +=
		*((A + i * acols) + k) * *((B + k * bcols) + j);
	    }
	}
    }
  pthread_exit (NULL);
}

//function which returns the time taken for a multi threaded process
unsigned long long
multi_thread_mm ()
{
  //allocating memory
  A = (int *) malloc (arows * acols * sizeof (int));
  B = (int *) malloc (brows * bcols * sizeof (int));
  C = (int *) malloc (crows * ccols * sizeof (int));

  pthread_t thread_id[4];

  struct timeval start, end;	//to compute the time 

  if (interactive == 1)
    {				//interactive
      printf ("Enter A:\n");
      fflush (stdout);
      input_matrix (A, arows, acols);
      printf ("Enter B:\n");
      fflush (stdout);
      input_matrix (B, brows, bcols);
    }
  else if (interactive == 0)
    {				//non interactive
      init_matrix (A, arows, acols);
      init_matrix (B, brows, bcols);
    }

  gettimeofday (&start, NULL);

  //creating the threads
  for (int i = 0; i < NUM_THREADS; i++)
    {
      pthread_create (&thread_id[i], NULL, (void *) matmul, NULL);
    }
  //joining or waiting for threads to terminate
  for (int i = 0; i < NUM_THREADS; i++)
    {
      pthread_join (thread_id[i], NULL);
    }

  gettimeofday (&end, NULL);

  //printing the output
  if (interactive == 1)
    {
      printf ("Result:\n");
      fflush (stdout);
      output_matrix (C, crows, ccols);
    }

  unsigned long long time = (end.tv_sec - start.tv_sec) * 1e6;
  time = (time + (end.tv_usec - start.tv_usec));
  //freeing the pointers
  free (A);
  free (B);
  free (C);

  return time;
}

//function which returns the time taken for a multi process
unsigned long long
multi_process_mm ()
{
  int n = crows, i = 0;
  pid_t pid[n];			//creating crows no.of child processes
  int *A, *B, *C;
  int s_id[3];

  //attaching segments
  s_id[0] =
    shmget (IPC_PRIVATE, sizeof (int) * arows * acols, IPC_CREAT | 0666);
  s_id[1] =
    shmget (IPC_PRIVATE, sizeof (int) * brows * bcols, IPC_CREAT | 0666);
  s_id[2] =
    shmget (IPC_PRIVATE, sizeof (int) * crows * ccols, IPC_CREAT | 0666);

  //attaching the arrays 
  A = (int *) shmat (s_id[0], NULL, 0);
  B = (int *) shmat (s_id[1], NULL, 0);
  C = (int *) shmat (s_id[2], NULL, 0);

  struct timeval start, end;

  if (interactive == 1)
    {				//interactive 
      printf ("Enter A:\n");
      fflush (stdout);
      input_matrix (A, arows, acols);
      printf ("Enter B:\n");
      fflush (stdout);
      input_matrix (B, brows, bcols);
    }
  else if (interactive == 0)
    {				//non interactive
      init_matrix (A, arows, acols);
      init_matrix (B, brows, bcols);
    }

  gettimeofday (&start, NULL);

  //computing the matrix multiplication
  for (int d = 0; d < n; d++)
    {
      pid[d] = fork ();
      if (pid[d] == 0)
	{
	  for (int j = 0; j < ccols; j++)
	    {
	      *((C + i * ccols) + j) = 0;
	      for (int k = 0; k < acols; k++)
		{
		  *((C + i * ccols) + j) +=
		    *((A + i * acols) + k) * *((B + k * bcols) + j);
		}
	    }
	  exit (0);
	}
      i++;
    }

  for (int d = 0; d < n; d++)
    {
      wait (NULL);
    }

  gettimeofday (&end, NULL);

  //printing the outputs 
  if (interactive == 1)
    {				//interactive
      printf ("Result:\n");
      fflush (stdout);
      output_matrix (C, crows, ccols);
    }

  unsigned long long time = (end.tv_sec - start.tv_sec) * 1e6;
  time = (time + (end.tv_usec - start.tv_usec));

  shmdt (A);			//detach
  shmdt (B);
  shmdt (C);
  shmctl (s_id[0], IPC_RMID, NULL);	//remove segment
  shmctl (s_id[1], IPC_RMID, NULL);
  shmctl (s_id[2], IPC_RMID, NULL);

  return time;
}

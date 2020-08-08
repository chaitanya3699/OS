/*
 * Program: Musical chairs game with n players and m intervals.
 * Author:  changeme  changeme
 * Roll# :  CS18BTECH11036, CS18BTECH11006
 */

#include <stdlib.h>     /* for exit, atoi */
#include <iostream>     /* for fprintf */
#include <errno.h>      /* for error code eg. E2BIG */
#include <getopt.h>     /* for getopt */
#include <assert.h>     /* for assert */
#include <chrono>       /* for timers */
#include <thread>
#include <chrono>
#include <string>
#include <string.h>
#include <condition_variable>
#include <unistd.h>
#define MAX 5000
/*
 * Forward declarations
 */

void usage (int argc, char *argv[]);
unsigned long long musical_chairs (int nplayers);

using namespace std;
int num;
int temp;
bool ready = false;
condition_variable cv1, cv2;
mutex m1, m2;
long long int Arr[MAX];

int
main (int argc, char *argv[])
{
  int c;
  int nplayers = 0;

  /* Loop through each option (and its's arguments) and populate variables */
  while (1)
    {
      int this_option_optind = optind ? optind : 1;
      int option_index = 0;
      static struct option long_options[] = {
    {"help", no_argument, 0, 'h'},
    {"nplayers", required_argument, 0, '1'},
    {0, 0, 0, 0}
      };

      c = getopt_long (argc, argv, "h1:", long_options, &option_index);
      if (c == -1)
    break;

      switch (c)
    {
    case 0:
      cerr << "option " << long_options[option_index].name;
      if (optarg)
        cerr << " with arg " << optarg << endl;
      break;

    case '1':
      nplayers = atoi (optarg);
      break;

    case 'h':
    case '?':
      usage (argc, argv);

    default:
      cerr << "?? getopt returned character code 0%o ??n" << c << endl;
      usage (argc, argv);
    }
    }

  if (optind != argc)
    {
      cerr << "Unexpected arguments.\n";
      usage (argc, argv);
    }


  if (nplayers == 0)
    {
      cerr << "Invalid nplayers argument." << endl;
      return EXIT_FAILURE;
    }

  unsigned long long game_time;
  game_time = musical_chairs (nplayers);

  cout << "Time taken for the game: " << game_time << " us" << endl;

  exit (EXIT_SUCCESS);
}

/*
 * Show usage of the program
 */
void
usage (int argc, char *argv[])
{
  cerr << "Usage:\n";
  cerr << argv[0] << "--nplayers <n>" << endl;
  exit (EXIT_FAILURE);
}

//umpir_main function to read the input and perform tasks 
void
umpire_main (int nplayers)
{
  cout << "Musical Chairs: " << nplayers << " player game with " << (nplayers
                                     -
                                     1) <<
    " laps." << endl;
  mutex m4;
  unique_lock < mutex > lock4 (m4);
  num = nplayers;
  lock4.unlock ();
  char s[100];

  /* read stdin only from umpire */
  while (1)
    {
      mutex m10;
      m10.lock ();
      if (num == 1)
    {
      m10.unlock ();
      break;
    }
      else
    {
      m10.unlock ();
    }

      cin >> s;         //taking input

      if (strcmp (s, "lap_start") == 0)
    {
      mutex m5;
      unique_lock < mutex > lock5 (m5);
      temp = 0;     //Updating temp as zero in each lap
      lock5.unlock ();
      //cv1.notify_all();
    }
      else if (strcmp (s, "music_start") == 0)
    {
      cv1.notify_all ();    //notifying the sleeping player threads
    }

      else if (strcmp (s, "umpire_sleep") == 0)
    {
      long long int t;
      cin >> t;
      std::this_thread::sleep_for (std::chrono::microseconds (t));  //umpire sleeps for the given time
    }
      else if (strcmp (s, "player_sleep") == 0)
    {
      int p_id;
      long long int t;
      cin >> p_id >> t;
      mutex m3;
      unique_lock < mutex > lock3 (m3);
      Arr[p_id] = t;    //array to store the times given for each player thread
      lock3.unlock ();
    }
      else if (strcmp (s, "music_stop") == 0)
    {
      cout << "======= lap# " << nplayers - (num -
                         1) << " =======" << endl;
      unique_lock < mutex > lock2 (m2);
      ready = true;     //making ready true for the players to enter
      cv2.wait (lock2);
    }
      else if (strcmp (s, "lap_stop") == 0)
    {
      mutex m6;

      unique_lock < mutex > lock6 (m6);
      num = num - 1;    //reducing number of players in each lap
      if (num == 1)
        {
          lock6.unlock ();
          cv1.notify_one ();
        }
      else
        {
          lock6.unlock ();
        }
    }
    }
  return;
}

void
player_main (int plid)
{
  while (1)
    {
      mutex m7, m8, m9;

      unique_lock < mutex > lock7 (m7);
      //When the only winner player is left
      if (num == 1)
    {
      lock7.unlock ();
      cout << "Winner is " << plid << endl;
      break;
    }
      else
    {
      lock7.unlock ();
    }

      unique_lock < mutex > lock8 (m8);
      if (Arr[plid] != 0)
    {
      lock8.unlock ();
      std::this_thread::sleep_for (std::chrono::microseconds (Arr[plid]));  //player sleeps for the given time
      Arr[plid] = 0;
    }
      else
    {
      lock8.unlock ();
    }

      unique_lock < mutex > lock9 (m9);
      //players enter when ready is made true
      if (ready)
    {
      lock9.unlock ();
      unique_lock < mutex > lock1 (m1);


      if (temp == num - 1)
        {

          cout << plid << " could not get chair\n";
          cout << "**********************" << endl;
          ready = false;
          cv2.notify_one ();    //notifying the waiting umpire after one lap is completed
          break;
        }

      temp++;       //increasing the count as a player occupies a chair

      cv1.wait (lock1);
    }
      else
    {
      lock9.unlock ();
    }

    }
  return;
}

unsigned long long
musical_chairs (int nplayers)
{
  auto t1 = chrono::steady_clock::now ();

  //spawn umpire thread
  thread u (umpire_main, nplayers);

  // Spawn n player threads.
  thread p[nplayers];
  for (int i = 0; i < nplayers; i++)
    {
      p[i] = thread (player_main, i);
    }


  for (int i = 0; i < nplayers; i++)
    {
      p[i].join ();
    }

  u.join ();

  auto t2 = chrono::steady_clock::now ();

  auto d1 = chrono::duration_cast < chrono::microseconds > (t2 - t1);

  return d1.count ();
}

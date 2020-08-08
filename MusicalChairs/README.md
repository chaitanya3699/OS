## Implementation of Musical Chairs using threads ##
- The procedure of game is as mentioned below.
  1. The n-1 chairs are arranged at random places in the ground.
  2. n players will be standing in the ground at random places.
  3. The game iterates n-1 times. In each of the ith lap,
   
        a. The umpire starts lap by ensuring that all players are ready.

        b. Once all players are ready the umpire starts the game by starting music.

        c. Each of the players keeps running around in the ground till music plays.

        d. When music is stopped by the umpire, each of the players quickly chooses a chair to occupy it. But if the chair is already occupied, the player steps back and chooses another free chair, and so on.

        e. One player will not get any chair as there is one less chair, so this player is out of game.

        f. The umpire stops the lap by ensuring that one player is knocked out, all other players are up from the chairs and a chair is removed from the game to start the next lap.

- In this program we create a thread for umpire and one thread for each of n players. The umpire thread works with all the players threads in lockstep synchronization. Each iteration, one player thread exits. Main thread waits for umpire thread and the n players to join back, and declares who has won the game. 

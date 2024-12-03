using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Input;

namespace GDAPS2_TeamA_Game
{
    // FSM for player sprite animations
    enum NPCState
    {
        FaceLeft,
        FaceRight,
        WalkLeft,
        WalkRight
    }

    /// <summary>
    /// Manages NPC objects (enemies) by controlling their movement, animations,
    /// and detecting their collisions with the player
    /// </summary>
    class NPC : GameObject
    {
        // Fields
        private Vector2 npcLoc;
        private NPCState state;
        private NPCState prevNPCState;
        private GameState gameState;

        // NPC spritesheet for drawing animations & chosen appearence
        private Texture2D spriteSheet;
        private int appearenceNum;
        private double stateDuration; // The NPC's time in each state
        private double walkDuration;
        private Random rng;

        // Animation fields
        private int frame;
        private double timePassed;
        private double fps;
        private double timePerFrame;
        private int npcRectTopY;

        // Constants for chosen frame rect in spritesheet
        private const int WalkFrameCount = 3;
        private const int NPCRectHeight = 32;
        private const int NPCRectWidth = 32;

        /// <summary>
        /// Get & set NPC's current location
        /// </summary>
        public float X
        {
            get
            {
                return this.npcLoc.X;
            }
            set
            {
                this.npcLoc.X = value;
            }
        }

        /// <summary>
        /// Get & set NPC's current animation state
        /// </summary>
        public NPCState State
        {
            get
            {
                return state;
            }
            set
            {
                state = value;
            }
        }

        /// <summary>
        /// Returns the current game state, and can be changed to correctly draw te npcs during the game state
        /// </summary>
        public GameState GameState
        {
            get
            {
                return gameState;
            }
            set
            {
                gameState = value;
            }
        }

        // Constructor that takes the spritesheet, the initial location, & the chosen appearence number
        public NPC(Texture2D spriteSheet, int x, int y, int width, int height, NPCState startingState, int appearenceNum)
                : base(spriteSheet, x, y, width, height)
        {
            this.spriteSheet = spriteSheet;
            npcLoc = new Vector2(x, y + 26);
            this.state = startingState;
            this.appearenceNum = appearenceNum;

            // Depending on the NPC, adjust their collision rectangle
            switch (appearenceNum)
            {
                // Luby
                case 1:
                    location.X += 8 * 4;
                    location.Y += 12 * 4 + 18; // pixel length, scaling factor, & adjustment for drawing
                    break;

                // Blup
                case 5:
                    location.X += 9 * 4;
                    location.Y += 14 * 4 + 18;
                    break;

                // Billow
                case 9:
                    location.X += 7 * 4;
                    location.Y += 9 * 4 + 18;
                    break;
            }

            // Initialize frame timing & starting location
            fps = 10.0;
            timePerFrame = 1.0 / fps;
            npcRectTopY = (32 * appearenceNum) + 1;
            rng = new Random();
            // Start test NPC's animations immediately by beginning with the required time
            stateDuration = 1;
            walkDuration = 1 + rng.NextDouble();
        }

        /// <summary>
        /// Update the NPC's animation according to the time passed
        /// </summary>
        /// <param name="gameTime">Time information</param>
        public void UpdateAnimation(GameTime gameTime)
        {
            // Update the time passed
            timePassed += gameTime.ElapsedGameTime.TotalSeconds;

            // If enough time has passed to animate a neew frame:
            if (timePassed >= timePerFrame)
            {
                // Change to the next image frame
                frame += 1;

                // If the walk cycle has ended, return to the 1st walking frame
                if (frame > WalkFrameCount)
                {
                    frame = 1;
                }

                // Remove the time it took to reset the time passed
                timePassed -= timePerFrame;
            }
        }

        /// <summary>
        /// Update the NPC's state
        /// </summary>
        /// <param name="gameTime">Tracks the current frame</param>
        public void Update(GameTime gameTime)
        {
            // Only animate NPC movement during gameplay
            if (this.GameState == GameState.Game)
            {
                // Animate the test NPC's state
                this.UpdateAnimation(gameTime);

                // Update the current amount of NPC animation time passed
                stateDuration += gameTime.ElapsedGameTime.TotalSeconds;

                // Check NPC's movement state & update it
                switch (this.State)
                {
                    // Left actions
                    case NPCState.FaceLeft:

                        // Change to walking state after 1 sec if previously turned around
                        if (stateDuration >= 1 && prevNPCState == NPCState.FaceRight)
                        {
                            // Reset the state duration
                            stateDuration -= 1;

                            // Set the new previous state & change the current state
                            prevNPCState = NPCState.FaceLeft;
                            this.State = NPCState.WalkLeft;
                        }
                        // Turn around after 1 sec if previously walking
                        else if (stateDuration >= 1 && prevNPCState == NPCState.WalkLeft)
                        {
                            // Reset the state duration
                            stateDuration -= 1;

                            // Set the new previous state & change the current state
                            prevNPCState = NPCState.FaceLeft;
                            this.State = NPCState.FaceRight;
                        }
                        break;

                    // Walk left
                    case NPCState.WalkLeft:
                        this.X -= 3;
                        location.X -= 3;

                        // Go back to standing after 1-2 secs of walking
                        if (stateDuration >= walkDuration)
                        {
                            // Reset the state duration
                            stateDuration -= walkDuration; 

                            // Set the new previous state & change back to facing left
                            prevNPCState = NPCState.WalkLeft;
                            this.State = NPCState.FaceLeft;
                        }
                        break;

                    // Right actions
                    case NPCState.FaceRight:
                        
                        // Change to walking state after 1 sec if previously turned around
                        if (stateDuration >= 1 && prevNPCState == NPCState.FaceLeft)
                        {
                            // Reset the state duration
                            stateDuration -= 1;

                            // Set the new previous state & change the current state
                            prevNPCState = NPCState.FaceRight;
                            this.State = NPCState.WalkRight;
                        }
                        // Turn around after 1 sec if previously walking
                        else if (stateDuration >= 1 && prevNPCState == NPCState.WalkRight)
                        {
                            // Reset the state duration
                            stateDuration -= 1;

                            // Set the new previous state & change the current state
                            prevNPCState = NPCState.FaceRight;
                            this.State = NPCState.FaceLeft;
                        }
                        break;

                    // Walk right
                    case NPCState.WalkRight:
                        this.X += 3;
                        location.X += 3;

                        // Go back to standing after 1-2 secs of walking
                        if (stateDuration >= walkDuration) 
                        {
                            // Reset the state duration
                            stateDuration -= walkDuration; 

                            // Set the new previous state & change back to facing right
                            prevNPCState = NPCState.WalkRight;
                            this.State = NPCState.FaceRight;
                        }
                        break;
                }
            }
        }

        /// <summary>
        /// Draw the NPC onto the given spritebatch based on its current state & frame 
        /// </summary>
        /// <param name="spriteBatch">Spritebatch to draw the current NPC to</param>
        public void Draw(SpriteBatch spriteBatch, WorldState w)
        {
            if (this.GameState == GameState.Game)
            {
                // Check the NPC's state to draw it
                switch (this.State)
                {
                    // Draw the NPC standing
                    case NPCState.FaceLeft:
                        // Flip the sprite to face left
                        DrawStanding(SpriteEffects.FlipHorizontally, spriteBatch, w);
                        break;

                    case NPCState.FaceRight:
                        DrawStanding(SpriteEffects.None, spriteBatch, w);
                        break;


                    // Draw the NPC walking
                    case NPCState.WalkLeft:
                        // Flip the sprite to face left
                        DrawWalking(SpriteEffects.FlipHorizontally, spriteBatch, w);
                        break;

                    case NPCState.WalkRight:
                        DrawWalking(SpriteEffects.None, spriteBatch, w);
                        break;
                }
            }
        }


        /// <summary>
        /// Draw the NPC standing (the 1st frame a.k.a. frame 0)
        /// </summary>
        /// <param name="flipSprite">
        /// Enum values for flipping the sprite horizontally &/or vertically
        /// </param>
        private void DrawStanding(SpriteEffects flipSprite, SpriteBatch spriteBatch, WorldState w)
        {
            switch (w)
            {
                case WorldState.Good:
                    spriteBatch.Draw(
                        spriteSheet,                    // The sprite's texture
                        npcLoc,                         // The location to draw on the screen
                        new Rectangle(                  // The "source" rect (the loc of the frame on the spritesheet)
                            64,                         // Start at 2 rect in for the X 
                            npcRectTopY,
                            NPCRectWidth,
                            NPCRectHeight),
                        Color.White,                    // The tint color
                        0,                              // No rotation
                        Vector2.Zero,                   // Origin inside the image (top left)
                        4f,                             // Scaled 400%
                        flipSprite,                     // Flip the image if necessary
                        0);                             // No layer depth
                    break;

                case WorldState.Evil:
                    spriteBatch.Draw(
                        spriteSheet,                    // The sprite's texture
                        npcLoc,                         // The location to draw on the screen
                        new Rectangle(                  // The "source" rect (the loc of the frame on the spritesheet)
                            64,                         // Start at 2 rect in for the X 
                            npcRectTopY,
                            NPCRectWidth,
                            NPCRectHeight),
                        Color.Red,                      // The tint color
                        0,                              // No rotation
                        Vector2.Zero,                   // Origin inside the image (top left)
                        4f,                             // Scaled 400%
                        flipSprite,                     // Flip the image if necessary
                        0);                             // No layer depth
                    break;
            }
        }

        /// <summary>
        /// Draw the NPC walking, based on the current Updated frame
        /// </summary>
        /// <param name="flipSprite">
        /// Enum values for flipping the sprite horizontally &/or vertically
        /// </param> 
        private void DrawWalking(SpriteEffects flipSprite, SpriteBatch spriteBatch, WorldState w)
        {
            switch (w)
            {
                case WorldState.Good:
                    spriteBatch.Draw(
                        spriteSheet,                    // The sprite's texture
                        npcLoc,                         // The location to draw on the screen
                        new Rectangle(                  // The "source" rect (the loc of the frame on the spritesheet)
                            64 + (frame * NPCRectWidth),    // Start at 2 rect in for the X 
                            npcRectTopY,
                            NPCRectWidth,
                            NPCRectHeight),
                        Color.White,                    // The tint color
                        0,                              // No rotation
                        Vector2.Zero,                   // Origin inside the image (top left)
                        4f,                             // Scaled 400%
                        flipSprite,                     // Flip the image if necessary
                        0);                             // No layer depth
                    break;

                case WorldState.Evil:
                    spriteBatch.Draw(
                        spriteSheet,                    // The sprite's texture
                        npcLoc,                         // The location to draw on the screen
                        new Rectangle(                  // The "source" rect (the loc of the frame on the spritesheet)
                            64 + (frame * NPCRectWidth),    // Start at 2 rect in for the X 
                            npcRectTopY,
                            NPCRectWidth,
                            NPCRectHeight),
                        Color.Red,                      // The tint color
                        0,                              // No rotation
                        Vector2.Zero,                   // Origin inside the image (top left)
                        4f,                             // Scaled 400%
                        flipSprite,                     // Flip the image if necessary
                        0);                             // No layer depth
                    break;
            }
        }

        /// <summary>
        /// Checks for player-npc collision
        /// </summary>
        /// <returns>True if the locations touch & false otherwise</returns>
        public bool Intersects(Player player)
        {
            return this.location.Intersects(player.Location);
        }
    }
}

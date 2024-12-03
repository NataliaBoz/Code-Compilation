// Loop through each object in gameObjects to check for collisions
foreach (GameObject obstacle in gameObjects)
{
    // Player-NPC Collision knock back
    // Player cannot take damage again until they touch a platform tile
    // (allows for a brief invulnerability buffer)
    if (obstacle is NPC && this.location.Intersects(obstacle.Location) && worldState == WorldState.Evil && !GodMode && !tookDamage)
    {
        isJumping = true;
        tookDamage = true;
        jumpingVelocity.Y = -23;
        TakeDamage();

        // Determine the direction to push the player back
        //Knockback for if the player is on the left of the NPC
        if (obstacle.Location.X >= this.location.X)
        {
            this.KnockBack = "left";
            Jump(gameObjects, gameTime);
        }
        //Knockback for if the player is on the right of the NPC
        if (obstacle.Location.X < this.location.X)
        {
            this.KnockBack = "right";
            Jump(gameObjects, gameTime);
        }
    }
}
# BuoyancyProject
![alt text](https://github.com/RodionKovalov/BuoyancyProject/blob/master/Photos/1.gif)
![alt text](https://github.com/RodionKovalov/BuoyancyProject/blob/master/Photos/2.gif)
## Overview:
The project implements basic realistic movement of the water surface with simulation of physical forces acting on objects in water, with convenient property settings. Developed on Unreal Engine 4.27.
At first, I decided to simulate the water surface using RenderTarget, but I came to the conclusion that it was too slow. Finding a pixel and processing its color takes too slowly, which is why the boat responded to the surface late, so I decided to do a full-fledged collision generation. Water surface simulation is based on UProceduralMeshComponent.
But now I’m faced with another problem: **the procedural mesh doesn’t support LOD**, so I had to implement my own system, but it’s still in development.
## Adjustment:
A component is embedded in the actor. The forces acting on the object act on certain points indicated in the pontoons array. The density of the object and the drag coefficient of the shape affect the behavior of the object in water. To adjust the balance you need to experiment with the property settings.
<div id="header" align="center">
  <img src="/Photos/BuoyancySettings.png"/>
  <img src="/Photos/BuoyancyPontoons.png"/>
</div>
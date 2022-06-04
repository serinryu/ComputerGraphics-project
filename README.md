# ComputerGraphics project using OpenGL

- Library : GLM, GLFW, GLEW, FreeType, Assimp
- Platform : Windows Microsoft Visual Studio
- Title: Find the Magic Pearl!


1.	요약 : 
제한 시간 안에 마법의 진주를 찾는 게임입니다. 프로그램이 실행됨과 동시에 시간이 카운트되고, 정해진 시간 안에 다이버가 보물 상자 위에 놓여져 있는 진주까지 도착해야 성공, 그렇지 않으면 실패입니다. 성공 하면, 다이버가 진주를 찾았으므로 상자 위에 있던 진주가 사라지고 성공 메세지가 출력됩니다. 실패하면, 진주는 여전히 상자 위에 남아있고 게임 실패 메세지가 출력됩니다. 최대한 빠르고, 지체 없이 다이버가 진주로 이동할 수 있도록 해야만 진주를 찾을 수 있습니다.


2. 사용법 : space 바를 누르면 다이버가 이동하고, 이동하고 있는 상태에서 space 바를 누르면 다이버가 정지합니다. 정해진 시간 안에 진주를 찾기 위해서는, 1) 프로그램 실행과 동시에 최대한 빠르게 space 바를 눌러 다이버를 움직이기 시작해야 하고, 2) 다이버가 이동하고 있는 중간에 space 바를 눌러 다이버를 멈추게 하여 시간을 지체하면 안됩니다. (추가적으로, 스크롤을 통해 거리를 조절할 수 있고, R key 를 누르면 카메라를 원위치로 이동시킬 수 있습니다. 또한, A key 를 누르면 보물 상자 위 진주를 회전시킬 수 있습니다.)

3. 시현 영상 : https://youtu.be/8CRGt0dUgjA

![image](https://user-images.githubusercontent.com/74564995/172014024-4502a9b2-f4a8-4188-90fd-66d00eeb45a6.png)

using OpenTK.Windowing.GraphicsLibraryFramework;
using OpenTK.Windowing;
using OpenTK.Graphics.OpenGL;
using OpenTK.Graphics;
using OpenTK.Input;
using OpenTK.Core;
using OpenTK;

namespace GolfGame
{
    public static unsafe class Program
    {
        private const int INIT_WINDOW_WIDTH = 1440;
        private const int INIT_WINDOW_HEIGHT = 900;


        public static int Main(string[] argv)
        {
            try
            {
                GLFW.Init();
                GLFW.WindowHint(WindowHintInt.ContextVersionMajor, 3);
                GLFW.WindowHint(WindowHintInt.ContextVersionMinor, 3);
                GLFW.WindowHint(WindowHintOpenGlProfile.OpenGlProfile, OpenGlProfile.Core);

                Window* window = GLFW.CreateWindow(INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT, "Golf Game", null, null);

                if (window == null)
                    return -1;

                GLFW.MakeContextCurrent(window);
                GLFW.SetFramebufferSizeCallback(window, (hwnd, width, height) => GL.Viewport(0, 0, width, height));
                GL.Viewport(0, 0, INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT);

                while (!GLFW.WindowShouldClose(window))
                {
                    GLFW.SwapBuffers(window);
                    GLFW.PollEvents();
                }
            }
            finally
            {
                GLFW.Terminate();
            }

            return 0;
        }
    }
}

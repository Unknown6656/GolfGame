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
        public static int Main(string[] argv)
        {
            try
            {
                GLFW.Init();
                GLFW.WindowHint(WindowHintInt.ContextVersionMajor, 3);
                GLFW.WindowHint(WindowHintInt.ContextVersionMinor, 3);
                GLFW.WindowHint(WindowHintOpenGlProfile.OpenGlProfile, OpenGlProfile.Core);

                Window* window = GLFW.CreateWindow(1440, 900, "Golf Game", null, null);

                if (window == null)
                    return -1;

                GLFW.MakeContextCurrent(window);
            }
            finally
            {
                GLFW.Terminate();
            }

            return 0;
        }
    }
}

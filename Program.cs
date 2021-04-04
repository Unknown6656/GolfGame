using OpenGL;
using GLFW;

using gl = OpenGL.GL;
using glfw = GLFW.Glfw;


namespace GolfGame
{
    public static class Program
    {
        public static int Main(string[] argv)
        {
            try
            {
                glfw.Init();
                glfw.WindowHint(Hint.ContextVersionMajor, 3);
                glfw.WindowHint(Hint.ContextVersionMinor, 3);
                glfw.WindowHint(Hint.OpenglProfile, Profile.Core);

                Window window = glfw.CreateWindow(1440, 900, "Golf Game", Monitor.None, Window.None);

                if (window == Window.None)
                    return -1;

                glfw.MakeContextCurrent(window);
            }
            finally
            {
                glfw.Terminate();
            }

            return 0;
        }
    }
}

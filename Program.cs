using System.Diagnostics;
using System;

using OpenTK.Windowing.GraphicsLibraryFramework;
using OpenTK.Windowing;
using OpenTK.Graphics.OpenGL;
using OpenTK.Graphics;
using OpenTK.Input;
using OpenTK.Core;
using OpenTK;
using OpenTK.Mathematics;

namespace GolfGame
{
    public static unsafe class Program
    {
        private const int INIT_WINDOW_WIDTH = 1440;
        private const int INIT_WINDOW_HEIGHT = 900;
        private static GolfCourse _course = new(Par.Par4, 420);


        public static int Main(string[] argv)
        {
            try
            {
                GLLoader.LoadBindings(new GLFWBindingsContext());

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


                (Vector2[] vertices, (int i1, int i2, int i3)[] indices) = _course.Rasterize();
                uint VBO;

                GL.GenBuffers(1, &VBO);
                GL.BindBuffer(BufferTargetARB.ArrayBuffer, VBO);
                GL.BufferData(BufferTargetARB.ArrayBuffer, )




                Stopwatch sw = new();

                while (!GLFW.WindowShouldClose(window))
                {
                    sw.Reset();
                    sw.Start();

                    ProcesInput(window);
                    RenderImage(window);

                    GLFW.SwapBuffers(window);
                    GLFW.PollEvents();

                    sw.Stop();

                    GLFW.SetWindowTitle(window, $"{(double)Stopwatch.Frequency / sw.ElapsedTicks,9:F3} FPS");
                }
            }
            finally
            {
                GLFW.Terminate();
            }

            return 0;
        }

        private static void ProcesInput(Window* window)
        {
            if (GLFW.GetKey(window, Keys.Escape) == InputAction.Press)
                GLFW.SetWindowShouldClose(window, true);
        }

        private static void RenderImage(Window* window)
        {
            GL.ClearColor(.2f, .3f, 1f, 1f);
            GL.Clear(ClearBufferMask.ColorBufferBit);
        }


        internal static float NextFloat(this Random random, float scale = 1f) => (float)random.NextDouble() * scale;
    }

    public sealed class GolfCourse
    {
        public Par Par { get; }
        public Random Random { get; }
        public Vector2 StartPosition { get; }
        public Vector2[] CourseMidwayPoints { get; }
        public (Vector2 Pos, float Size) PuttingGreen { get; }


        public GolfCourse(Par par, int? random_seed)
        {
            Par = par;
            Random = random_seed is int seed ? new(seed) : new();

            float m = .39f - (par - Par.Par3) * .13f;

            StartPosition = new(Random.NextFloat(m), Random.NextFloat());
            PuttingGreen = (new(Random.NextFloat(m) + (1 - m), Random.NextFloat()), Random.NextFloat(.15f);

            double φ = Math.Atan2(PuttingGreen.Pos.Y - StartPosition.Y, PuttingGreen.Pos.X - StartPosition.X) + Math.PI / 2;
            Vector2 lerp(float f) => new(f * StartPosition.X + (1 - f) * PuttingGreen.Pos.X, f * StartPosition.Y + (1 - f) * PuttingGreen.Pos.Y);

            if (par is Par.Par4)
            {
                float p = Random.NextFloat(.6f) + .2f;
                float d = Random.NextFloat(.5f) - .25f;
                Vector2 mid = lerp(p);

                mid.X += (float)Math.Cos(φ) * d;
                mid.Y += (float)Math.Sin(φ) * d;

                CourseMidwayPoints = new[] { mid };
            }
            else if (par is Par.Par5)
            {
                float d1 = Random.NextFloat(.5f) - .25f;
                float d2 = Random.NextFloat(.5f) - .25f;
                Vector2 mid1 = lerp(.3f);
                Vector2 mid2 = lerp(.6f);

                mid1.X += (float)Math.Cos(φ) * d1;
                mid1.Y += (float)Math.Sin(φ) * d1;
                mid2.X += (float)Math.Cos(φ) * d2;
                mid2.Y += (float)Math.Sin(φ) * d2;

                CourseMidwayPoints = new[] { mid1, mid2 };
            }
            else
                CourseMidwayPoints = Array.Empty<Vector2>();
        }

        public GolfCourseVectorBuffer Rasterize()
        {
            GolfCourseVectorBuffer vbo = new()
            {
                VBOIndex = GL.GenBuffer(),
            };

            GL.BindBuffer(BufferTargetARB.ArrayBuffer, vbo.VBOIndex);
            GL.BufferData(BufferTargetARB.ArrayBuffer, vbo.Vertices, BufferUsageARB.StaticDraw); // DynamicDraw
            GL.BindBuffer(BufferTargetARB.ArrayBuffer, 0);


            GL.Enable(EnableCap.VertexArray);
            GL.BindBuffer(BufferTargetARB.ArrayBuffer, vbo.VBOIndex);
            GL.vertexpointer


            return vbo;
        }
    }

    public sealed class GolfCourseVectorBuffer
    {
        public uint VBOIndex { get; init; }
        public Vector2[] Vertices { get; init; }
        public (int i1, int i2, int i3)[] Indices { get; init; }
    }

    public enum Par
    {
        Par3 = 0,
        Par4 = 1,
        Par5 = 2,
    }
}

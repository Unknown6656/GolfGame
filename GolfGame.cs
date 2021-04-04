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
using OpenTK.Windowing.Desktop;
using OpenTK.Windowing.Common;
using Unknown6656.Mathematics.Numerics;
using System.Collections.Generic;
using Unknown6656.Common;

namespace GolfGame
{
    public unsafe record BufferObject(uint ID)
        : IDisposable
    {
        public BufferObject SetData<T>(Span<T> values, bool clear = false)
            where T : unmanaged
        {
            GL.BindBuffer(BufferTargetARB.ArrayBuffer, ID);
            GL.BufferData(BufferTargetARB.ArrayBuffer, values, BufferUsageARB.StaticDraw);

            if (clear)
                GL.BindBuffer(BufferTargetARB.ArrayBuffer, 0);

            return this;
        }

        public void Dispose()
        {
            GL.BindBuffer(BufferTargetARB.ArrayBuffer, 0);
            GL.DeleteBuffer(ID);
        }

        public static BufferObject Create() => new(GL.GenBuffer());
    }

    public sealed class GolfGame
        : GameWindow
    {
        private const int INIT_WINDOW_WIDTH = 1440;
        private const int INIT_WINDOW_HEIGHT = 900;

        private static GolfCourse? _curent_golf_course = new(Par.Par4, 420);
        private readonly List<BufferObject> _vbos = new();


        public GolfGame()
            : base(
                new GameWindowSettings {
                    IsMultiThreaded = true,
                    RenderFrequency = 0,
                    UpdateFrequency = 0,
                },
                new NativeWindowSettings {
                    API = ContextAPI.OpenGL,
                    APIVersion = new(4, 6),
                    Size = new(INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT),
                    NumberOfSamples = 4,
                    Title = "Golf Game",
#if DEBUG
                    Flags = ContextFlags.Debug
#endif
                })
        {
        }

        protected override void OnLoad()
        {
            GL.ClearColor(.3f, .5f, 1f, 1f);

            base.OnLoad();
        }

        protected override void OnUnload()
        {
            _vbos.Do(obj => obj.Dispose());
            _vbos.Clear();

            base.OnUnload();
        }

        protected override void OnClosed()
        {
            GLFW.Terminate();

            base.OnClosed();
        }

        //protected override void OnResize()
        //{
        //    GL.Viewport(0, 0, Width, Height);
        //
        //    base.OnResize();
        //}

        protected override void OnKeyUp(KeyboardKeyEventArgs e)
        {
            if (e.Key is Keys.Escape)
                Close();

            base.OnKeyUp(e);
        }

        protected override void OnRenderFrame(FrameEventArgs e)
        {
            RenderImage();
            SwapBuffers();
#if DEBUG
            Title = $"{1 / e.Time,9:F3} FPS";
#endif
            base.OnRenderFrame(e);
        }

        private void RenderImage()
        {
            GL.Clear(ClearBufferMask.ColorBufferBit);

            // TODO
        }

        public static void Main(string[] argv)
        {
            using (GolfGame game = new())
                game.Run();
        }
    }

    internal static class Util
    {
        public static float NextFloat(this XorShift random, float scale) => random.NextFloat() * scale;
    }

    public sealed class GolfCourse
    {
        public Par Par { get; }
        public XorShift Random { get; }
        public Vector2 StartPosition { get; }
        public Vector2[] CourseMidwayPoints { get; }
        public (Vector2 Pos, float Size) PuttingGreen { get; }


        public GolfCourse(Par par, int? random_seed)
        {
            Par = par;
            Random = random_seed is int seed ? new(seed) : new();

            float m = .39f - (par - Par.Par3) * .13f;

            StartPosition = new(Random.NextFloat(m), Random.NextFloat());
            PuttingGreen = (new(Random.NextFloat(m) + (1 - m), Random.NextFloat()), Random.NextFloat(.15f));

            double φ = Math.Atan2(PuttingGreen.Pos.Y - StartPosition.Y, PuttingGreen.Pos.X - StartPosition.X) + Math.PI * .5;
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
            //GL.vertexpointer


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

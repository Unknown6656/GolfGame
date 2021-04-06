using System.Collections.Generic;
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
using Unknown6656.Common;
using System.Threading.Tasks.Sources;
using System.Runtime.InteropServices;
using System.Runtime.CompilerServices;
using System.Reflection;
using System.Threading.Tasks;
using System.Runtime.ExceptionServices;

namespace GolfGame
{
    public sealed class GolfGame
        : GameWindow
    {
        private const int INIT_WINDOW_WIDTH = 1440;
        private const int INIT_WINDOW_HEIGHT = 900;

        private static GolfCourse? _curent_golf_course = new(Par.Par4, 420);
        private readonly Dictionary<string, (int Location, UniformType Type)> _uniforms = new();
        private readonly Dictionary<string, (uint VBO, uint VAO, uint EBO, uint attribute, Array vertices, Array indices)> _attributes = new();
        private bool _loaded = true;

        public uint ProgramHandle { get; private set; }


        public GolfGame()
            : base(
                new GameWindowSettings {
                    IsMultiThreaded = true,
                    RenderFrequency = 0,
                    UpdateFrequency = 0,
                },
                new NativeWindowSettings {
                    API = ContextAPI.OpenGL,
                    APIVersion = new(3,3),
                    Size = new(INIT_WINDOW_WIDTH, INIT_WINDOW_HEIGHT),
                    NumberOfSamples = 4,
                    Title = "Golf Game",
#if DEBUG
                    Flags = ContextFlags.Debug
#endif
                })
        {
            unsafe
            {
                GLFW.MakeContextCurrent(WindowPtr);
            }

            Console.WriteLine(GL.GetString(StringName.Renderer));
            Console.WriteLine(GL.GetString(StringName.Version));
            Console.WriteLine(GL.GetString(StringName.ShadingLanguageVersion));
            Console.WriteLine(GL.GetString(StringName.Vendor));

            GL.Enable(EnableCap.DebugOutput);

            Task.Factory.StartNew(async delegate
            {
                try
                {
                    while (_loaded)
                    {
                        uint uid = 0;
                        int length = -1;
                        DebugType type = default;
                        DebugSource source = default;
                        DebugSeverity severity = default;
                        uint count = GL.GetDebugMessageLog(1, 1024, ref source, ref type, ref uid, ref severity, ref length, out string message);

                        if (count > 0 || length >= 0)
                            Console.WriteLine($"{uid:x8}: {source} {type} {severity} | {GL.GetError()} | {source}");
                        else
                            await Task.Delay(50);
                    }
                }
                catch (Exception ex)
                {
                    Console.WriteLine(ex);
                }
            });
        }

        protected override unsafe void OnLoad()
        {
            GL.Enable(EnableCap.VertexArray);

            ProgramHandle = GL.CreateProgram();

            using Shader s_vert = new("shaders/shader.vert", ShaderType.VertexShader);
            using Shader s_frag = new("shaders/shader.frag", ShaderType.FragmentShader);
            int uniform_count;

            Console.WriteLine(s_vert.CompileLog);
            Console.WriteLine(s_frag.CompileLog);

            GL.AttachShader(ProgramHandle, s_vert);
            GL.AttachShader(ProgramHandle, s_frag);
            GL.LinkProgram(ProgramHandle);
            GL.UseProgram(ProgramHandle);
            GL.DetachShader(ProgramHandle, s_vert);
            GL.DetachShader(ProgramHandle, s_frag);
            GL.GetProgramiv(ProgramHandle, ProgramPropertyARB.ActiveUniforms, &uniform_count);

            for (uint i = 0; i < uniform_count; i++)
            {
                UniformType type = default;
                int length = 0, size = 0;
                string key = GL.GetActiveUniform(ProgramHandle, i, 1024, ref length, ref size, ref type);
                int location = GL.GetUniformLocation(ProgramHandle, key);

                _uniforms[key] = (location, type);
            }

            GL.ClearColor(.3f, .5f, 1f, 1f);


            (Vector3[] vertices, uint[] indices) = _curent_golf_course.Rasterize(1);

            ProvideVBO("aPosition", vertices, indices);

            base.OnLoad();
        }

        private unsafe (uint VBO, uint VAO, uint EBO, uint attribute) ProvideVBO<T>(string name, T[] array, uint[] indices)
            where T : unmanaged
        {
            uint vbo, vao = 0, ebo = 0, attribute;

            if (_attributes.TryGetValue(name, out var entry))
                (vbo, vao, ebo, attribute) = (entry.VBO, entry.VAO, entry.EBO, entry.attribute);
            else
            {
                attribute = (uint)GL.GetAttribLocation(ProgramHandle, name);
                vbo = GL.GenBuffer();
            }

            GL.BindBuffer(BufferTargetARB.ArrayBuffer, vbo);
            GL.BufferData(BufferTargetARB.ArrayBuffer, array, BufferUsageARB.StaticDraw); // DynamicDraw

            if (vao == 0)
                vao = GL.GenVertexArray();

            GL.BindVertexArray(vao);
            GL.VertexAttribPointer(attribute, sizeof(T) / sizeof(float), VertexAttribPointerType.Float, false, sizeof(T), null);
            GL.EnableVertexAttribArray(attribute);

            if (ebo == 0)
                ebo = GL.GenBuffer();

            GL.BindBuffer(BufferTargetARB.ElementArrayBuffer, ebo);
            GL.BufferData(BufferTargetARB.ElementArrayBuffer, indices, BufferUsageARB.StaticDraw);
            GL.ObjectLabel(ObjectIdentifier.Buffer, vbo, name.Length + 4, name + "-vbo");
            GL.ObjectLabel(ObjectIdentifier.VertexArray, vao, name.Length + 4, name + "-vao");
            GL.ObjectLabel(ObjectIdentifier.Buffer, ebo, name.Length + 4, name + "-ebo");

            _attributes[name] = (vbo, vao, ebo, attribute, array, indices);

            return (vbo, vao, ebo, attribute);
        }

        protected override void OnUnload()
        {
            GL.UseProgram(0);

            foreach ((uint vbo, uint vao, uint ebo, _, _, _) in _attributes.Values)
            {
                GL.BindBuffer(BufferTargetARB.ArrayBuffer, 0);
                GL.BindVertexArray(0);
                GL.DeleteBuffer(vbo);
                GL.DeleteBuffer(ebo);
                GL.DeleteVertexArray(vao);
            }

            GL.DeleteProgram(ProgramHandle);

            _attributes.Clear();

            base.OnUnload();
        }

        protected override void OnClosed()
        {
            GLFW.Terminate();

            _loaded = false;

            base.OnClosed();
        }

        protected override void OnResize(ResizeEventArgs e)
        {
            GL.Viewport(0, 0, Size.X, Size.Y);

            base.OnResize(e);
        }

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
            GL.UseProgram(ProgramHandle);

            foreach ((_, (uint vbo, uint vao, uint ebo, _, Array vertices, Array indices)) in _attributes)
            {
                //GL.BindBuffer(BufferTargetARB.ArrayBuffer, vbo);
                //GL.BindBuffer(BufferTargetARB.ElementArrayBuffer, ebo);
                GL.BindVertexArray(vao);
                //GL.DrawArrays(PrimitiveType.Triangles, 0, vertices.Length);
                GL.DrawElements(PrimitiveType.Triangles, indices.Length, DrawElementsType.UnsignedInt, 0);
            }

            // TODO
        }

        public static void Main(string[] argv)
        {
            using (GolfGame game = new())
                game.Run();
        }
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

        public (Vector3[] vertices, uint[] indices) Rasterize(int size)
        {
            Vector3[] vertices = new Vector3[(size + 1) * (size + 1)];
            uint[] indices = new uint[size * size * 6];

            for (int i = 0; i < vertices.Length; ++i)
            {
                int ix = i % (size + 1);
                int iz = i / (size + 1);
                float x = (float)ix / size;
                float z = (float)iz / size;
                float y = 0f;

                vertices[i] = new(x, z, y);

                if (ix < size && iz < size)
                {
                    indices[(ix + iz * size) * 6 + 0] = (uint)(ix + iz * (size + 1));
                    indices[(ix + iz * size) * 6 + 1] =
                    indices[(ix + iz * size) * 6 + 3] = (uint)(ix + 1 + iz * (size + 1));
                    indices[(ix + iz * size) * 6 + 4] = (uint)(ix + 1 + (iz + 1) * (size + 1));
                    indices[(ix + iz * size) * 6 + 2] =
                    indices[(ix + iz * size) * 6 + 5] = (uint)(ix + (iz + 1) * (size + 1));
                }
            }

            return (vertices, indices);
        }
    }

    public enum Par
    {
        Par3 = 0,
        Par4 = 1,
        Par5 = 2,
    }
}

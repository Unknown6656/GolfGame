using System.IO;
using System;

using OpenTK.Graphics.OpenGL;

using Unknown6656.Mathematics.Numerics;

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

    public sealed class Shader
    {
        public uint Handle { get; }
        public string Path { get; }
        public string SourceCode { get; }
        public string? CompileLog { get; }
        public ShaderType ShaderType { get; }


        public Shader(string path, ShaderType type)
        {
            Path = path;
            SourceCode = File.ReadAllText(path);
            Handle = GL.CreateShader(type);

            GL.ShaderSource(Handle, SourceCode);
            GL.CompileShader(Handle);

            int log_length = 0;
            string log = GL.GetShaderInfoLog(Handle, 1024 * 1024, ref log_length);

            if (!string.IsNullOrEmpty(log))
                CompileLog = log;
        }
    }

    internal static class Util
    {
        public static float NextFloat(this XorShift random, float scale) => random.NextFloat() * scale;
    }
}

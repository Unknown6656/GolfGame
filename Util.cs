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
        : IDisposable
    {
        public uint Handle { get; }
        public string Path { get; }
        public string SourceCode { get; }
        public string? CompileLog { get; }
        public ShaderType ShaderType { get; }


        public Shader(string path, ShaderType type)
        {
            Path = path;
            ShaderType = type;
            SourceCode = File.ReadAllText(path);
            Handle = GL.CreateShader(type);

            GL.ShaderSource(Handle, SourceCode);
            GL.CompileShader(Handle);
            GL.ObjectLabel(ObjectIdentifier.Shader, Handle, path.Length, path);

            int log_length = 0;
            string log = GL.GetShaderInfoLog(Handle, 1024 * 1024, ref log_length);

            if (!string.IsNullOrEmpty(log))
                CompileLog = log;
        }

        public override bool Equals(object? obj) => obj is Shader s && Handle == s.Handle;

        public override int GetHashCode() => Handle.GetHashCode();

        public override string ToString() => $"{ShaderType}: {Handle}";

        public void Dispose() => GL.DeleteShader(Handle);

        public static implicit operator uint(Shader shader) => shader.Handle;
    }

    internal static class Util
    {
        public static float NextFloat(this XorShift random, float scale) => random.NextFloat() * scale;
    }
}

using System;
using System.Runtime.InteropServices;

namespace ArcaneScriptDemo
{
    // Matching C enum values (adjust as needed)
    public enum ValueType
    {
        VAL_INT,
        VAL_STRING,
        VAL_BOOL,
        VAL_NULL,
        VAL_ERROR
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Value
    {
        public ValueType type;
        public int temp;
        public int int_val;
        public IntPtr str_val; // For VAL_STRING and VAL_ERROR

        public string GetString()
        {
            return str_val != IntPtr.Zero ? Marshal.PtrToStringAnsi(str_val) : string.Empty;
        }
    }

    public class ArcaneInterop
    {
        // Adjust the DLL name (or path) as needed.
        [DllImport("arcane.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern Value interpret(string src);
    }

    class Program
    {
        static void Main(string[] args)
        {
            // Sample script to interpret.
            string script = "print(\"Hello, ArcaneScript World!\");";
            Value result = ArcaneInterop.interpret(script);

            // Check result type and handle accordingly.
            switch (result.type)
            {
                case ValueType.VAL_INT:
                    Console.WriteLine("Returned int: " + result.int_val);
                    break;
                case ValueType.VAL_STRING:
                case ValueType.VAL_ERROR:
                    Console.WriteLine("Returned string: " + result.GetString());
                    break;
                default:
                    Console.WriteLine("Returned value of unknown type.");
                    break;
            }
        }
    }
}
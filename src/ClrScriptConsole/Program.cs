using System.Runtime.InteropServices;

namespace ClrScriptConsole
{
    internal class Program
    {
        [DllImport(
            "arcane_script.dll",
            CharSet = CharSet.Unicode)]
        internal static extern ApeType ape_make();

        static void Main(string[] args)
        {
            var vm = ape_make();
            Console.WriteLine("Hello, World!");
        }
    }
}
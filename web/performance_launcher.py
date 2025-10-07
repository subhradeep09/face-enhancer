#!/usr/bin/env python3
"""
Performance Comparison and Server Launcher
Choose between different enhancement modes based on your needs
"""

import sys
import subprocess
import time

def print_banner():
    """Print application banner"""
    print("\n" + "="*80)
    print("🎭 GFPGAN Face Enhancer - Performance Modes")
    print("="*80)
    print()

def print_performance_comparison():
    """Show performance comparison table"""
    print("📊 PERFORMANCE COMPARISON:")
    print()
    print("┌─────────────────────┬─────────────────┬─────────────────┬─────────────────┐")
    print("│ Feature             │ Original Server │ Enhanced Server │ Fast Server     │")
    print("├─────────────────────┼─────────────────┼─────────────────┼─────────────────┤")
    print("│ Processing Speed    │ Slow (10-30s)  │ Medium (3-8s)   │ Ultra Fast (<2s)│")
    print("│ Quality             │ High            │ High            │ Good            │")
    print("│ Face Enhancement    │ Yes             │ Yes (Optional)  │ No              │")
    print("│ Noise Reduction     │ Advanced        │ Optimized       │ Skipped         │")
    print("│ Upscaling Quality   │ LANCZOS4        │ LANCZOS4        │ Linear (Fast)   │")
    print("│ Memory Usage        │ High            │ Medium          │ Low             │")
    print("│ CPU Usage           │ High            │ Medium          │ Low             │")
    print("│ Best For            │ Quality         │ Balance         │ Speed           │")
    print("└─────────────────────┴─────────────────┴─────────────────┴─────────────────┘")
    print()

def print_optimization_details():
    """Show what optimizations were made"""
    print("🔧 OPTIMIZATIONS MADE:")
    print()
    print("🚀 Enhanced Server Optimizations:")
    print("   • Removed slow fastNlMeansDenoisingColored() function")
    print("   • Reduced bilateral filter parameters (9→7, 80→40)")
    print("   • Optimized face detection parameters (1.1→1.2 scale)")
    print("   • Reduced CLAHE tile grid size (8×8 → 4×4)")
    print("   • Smaller gaussian blur kernels (21×21 → 11×11)")
    print("   • Added optional face enhancement toggle")
    print("   • Performance timing for each step")
    print()
    print("⚡ Fast Server Optimizations:")
    print("   • Uses INTER_LINEAR instead of LANCZOS4 for upscaling")
    print("   • Skips all noise reduction entirely")
    print("   • Skips histogram equalization")
    print("   • Skips face detection and enhancement")
    print("   • Simple kernel sharpening only")
    print("   • Lower JPEG quality (90 instead of 95)")
    print("   • Minimal parameter parsing")
    print()

def get_user_choice():
    """Get user's server choice"""
    print("🎯 CHOOSE YOUR ENHANCEMENT MODE:")
    print()
    print("1. 🐌 Original Server    - Best quality, slowest speed")
    print("2. ⚖️  Enhanced Server   - Balanced quality and speed (RECOMMENDED)")
    print("3. ⚡ Fast Server       - Fastest speed, good quality")
    print("4. 📊 Show detailed comparison")
    print("5. ❌ Exit")
    print()
    
    while True:
        try:
            choice = input("Enter your choice (1-5): ").strip()
            if choice in ['1', '2', '3', '4', '5']:
                return choice
            else:
                print("❌ Invalid choice. Please enter 1, 2, 3, 4, or 5.")
        except KeyboardInterrupt:
            print("\n\n👋 Goodbye!")
            sys.exit(0)

def run_server(server_type):
    """Run the selected server"""
    servers = {
        '1': {
            'name': 'Original Server',
            'file': 'gfpgan_web_server.py',
            'port': 8080,
            'description': 'High quality, slower processing'
        },
        '2': {
            'name': 'Enhanced Server',
            'file': 'enhanced_server.py',
            'port': 8081,
            'description': 'Optimized for balanced performance'
        },
        '3': {
            'name': 'Fast Server',
            'file': 'fast_enhanced_server.py',
            'port': 8082,
            'description': 'Ultra-fast processing'
        }
    }
    
    server = servers[server_type]
    
    print(f"\n🚀 Starting {server['name']}...")
    print(f"📝 Description: {server['description']}")
    print(f"🌐 URL: http://localhost:{server['port']}/")
    print()
    print("⏳ Starting server...")
    
    try:
        # Run the selected server
        subprocess.run([sys.executable, server['file']], cwd='.')
    except KeyboardInterrupt:
        print(f"\n🛑 {server['name']} stopped by user")
    except FileNotFoundError:
        print(f"❌ Error: {server['file']} not found!")
        print("Make sure you're running this from the correct directory.")
    except Exception as e:
        print(f"❌ Error starting server: {e}")

def main():
    """Main application"""
    print_banner()
    print_performance_comparison()
    
    while True:
        choice = get_user_choice()
        
        if choice == '4':
            print_optimization_details()
            input("\nPress Enter to continue...")
            print_performance_comparison()
            continue
        elif choice == '5':
            print("👋 Goodbye!")
            break
        elif choice in ['1', '2', '3']:
            run_server(choice)
            
            # Ask if user wants to try another server
            print("\n" + "="*50)
            try_again = input("Would you like to try another server? (y/n): ").strip().lower()
            if try_again not in ['y', 'yes']:
                print("👋 Goodbye!")
                break
        
        print()

if __name__ == "__main__":
    main()
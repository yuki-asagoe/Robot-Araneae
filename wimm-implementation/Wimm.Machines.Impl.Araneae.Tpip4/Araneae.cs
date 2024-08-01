using System.Collections.Immutable;
using Wimm.Common;
using Wimm.Machines.Impl.Araneae.Tpip4.Component;
using Wimm.Machines.TpipForRasberryPi;
using Wimm.Machines.TpipForRasberryPi.Import;

namespace Wimm.Machines.Impl.Araneae.Tpip4
{
    [LoadTarget]
    public class Araneae : TpipForRasberryPiMachine
    {
        public override string Name => "アレイニー";
        internal ImmutableArray<MotorDriver> MotorDrivers { get; }
        private Timer? PeriodicTimer { get; }
        public Araneae(MachineConstructorArgs? args) : base(args)
        {
            Camera = new Tpip4Camera("フロント", "バック", "アーム");
            if (args is not null && Camera is Tpip4Camera camera) { Hwnd?.AddHook(camera.WndProc); }
            MotorDrivers = [
                new MotorDriver(0x55),
                new MotorDriver(0x56)
            ];
            if (args is not null)
            {
                PeriodicTimer = new Timer(HandleTimer, null, 3000, 3000);
            }
            

            Information = [
                new InformationNode("MotorDriver",
                    MotorDrivers.Select(it => 
                        new InformationNode(
                            it.I2CAddress.ToString(), []
                        )
                    ).ToImmutableArray()
                )
            ];
            
            {
                Func<double> speedModifier = () => SpeedModifier;
                StructuredModules = new ModuleGroup(
                    "modules",
                    [
                        new ModuleGroup(
                            "crawlers",
                            [],
                            [
                                new AraneaeMotor("left", "機動用左クローラー", MotorDrivers[0],1,speedModifier),
                                new AraneaeMotor("right", "機動用右クローラー", MotorDrivers[0],2,speedModifier)
                            ]
                        ),
                        new ModuleGroup(
                            "container",
                            [],
                            [
                                new AraneaeMotor("string_puller", "救助機構せりだしモーター2", MotorDrivers[0], 0, speedModifier),
                                //new AraneaeMotor("protruder1", "救助機構せりだしモーター1", MotorDrivers[0], 3, speedModifier),
                                //new AraneaeMotor("protruder2", "救助機構せりだしモーター2", MotorDrivers[0], ?, speedModifier)
                            ]
                        )
                    ],
                    []
                );
            }
        }
        public override void Dispose()
        {
            base.Dispose();
            PeriodicTimer?.Dispose();
        }
        private void HandleTimer(object? _)
        {
            int size = 0;
            int address = 0;
            byte[] buf = new byte[32];
            for(int i=0;i<MotorDrivers.Length;i++)
            {
                var m = MotorDrivers[i];
                TPJT4.NativeMethods.Req_Recv_I2Cdata(0, m.I2CAddress, 1);
                int received = TPJT4.NativeMethods.Recv_I2Cdata(0, buf, ref address, ref size);
                Information[0].Entries[i].Value = received != 0 ? "Online" : "Offline";
            }
        }
        protected override ControlProcess StartControlProcess()
        {
            return new AraneaeControlProcess(MotorDrivers);
        }

        internal class AraneaeControlProcess(IEnumerable<MotorDriver> motorDrivers) : ControlProcess
        {
            private IEnumerable<MotorDriver> MotorDrivers = motorDrivers;
            public override void Dispose()
            {
                base.Dispose();
                foreach(var i in MotorDrivers)
                {
                    i.SendAll();
                }
            }
        }
    }
}

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Wimm.Common;
using Wimm.Machines.Component;

namespace Wimm.Machines.Impl.Araneae.Tpip4.Component
{
    internal class AraneaeMotor : Motor
    {
        public AraneaeMotor(string name, string description, MotorDriver host, byte motorID, Func<double> speedModifierProvider) : base(name, description)
        {
            HostDriver = host;
            MotorID = motorID;
            SpeedModifierProvider = speedModifierProvider;
            Features = [
                .. Features,
                new Feature<Delegate>(
                    "stop",
                    "モーターの回転を停止します",
                    Stop
                ),
                new Feature<Delegate>(
                    "brake",
                    "モーターにショートブレーキをかけます",
                    Brake
                ),
            ];
        }
        private MotorDriver HostDriver { get; }
        private byte MotorID { get; }
        private Func<double> SpeedModifierProvider { get; }

        public override Feature<Action<double>> RotationFeature 
            => new Feature<Action<double>>(
                Motor.RotationFeatureName,
                "モーターを回転させます。\n\n[引数]\n- double speed - 範囲 -1 ~ 1 です。",
                Rotation
            );
        public void Rotation(double speed)
        {
            double speedModifier = SpeedModifierProvider();
            MotorDriver.MotorMode mode;
            if (Math.Abs(speedModifier) < 0.0001)
            {
                mode = MotorDriver.MotorMode.Stop;
            }
            else if (speedModifier > 0)
            {
                mode = MotorDriver.MotorMode.Drive;
            }
            else
            {
                mode = MotorDriver.MotorMode.ReverseDrive;
            }
            speedModifier = Math.Clamp(speedModifier, 0, 1);
            HostDriver.AddMessage(new MotorDriver.Message(MotorID,mode, (byte)(speedModifier * 255)));
        }
        public void Brake()
        {
            HostDriver.AddMessage(new MotorDriver.Message(MotorID, MotorDriver.MotorMode.Brake, 0));
        }
        public void Stop()
        {
            HostDriver.AddMessage(new MotorDriver.Message(MotorID, MotorDriver.MotorMode.Stop, 0));
        }

        public override string ModuleName => "アレイニー モーター";

        public override string ModuleDescription => "アレイニーの汎用モータ";
    }
}

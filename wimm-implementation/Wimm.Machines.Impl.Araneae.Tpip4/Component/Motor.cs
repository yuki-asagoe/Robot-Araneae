using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Wimm.Common;
using Wimm.Machines.Component;
using Wimm.Machines.TpipForRasberryPi.Import;

namespace Wimm.Machines.Impl.Araneae.Tpip4.Component
{
    internal class AraneaeSubMotor : Motor
    {
        public AraneaeSubMotor(string name, string description,DigitalPin pin1,DigitalPin pin2,Araneae host) : base(name, description)
        {
            this.Pin1 = pin1;
            this.Pin2 = pin2;
            host.OnSetControl += SetControl;
        }
        DigitalPin Pin1 { get; set; }
        DigitalPin Pin2 { get; set; }
        MotorMode Mode { get; set; } = MotorMode.Stop;
        public override Feature<Action<double>> RotationFeature => 
            new Feature<Action<double>>(
                Motor.RotationFeatureName,
                "モータを回転させます\n[引数]\n- double speed : [1 ~ -1]",
                RotationImpl
            );

        public override string ModuleName => "アレイニー 補助モーター";

        public override string ModuleDescription => "TPIP出力を使用するサブモータ";
        private TPJT4.OUT_DT_STR SetControl(TPJT4.OUT_DT_STR outData)
        {
            switch (Mode)
            {
                case MotorMode.Stop:
                    break;
                case MotorMode.Brake:
                    outData.d_out |= (ushort)((0b1 << ((byte)Pin1)) | (0b1 << ((byte)Pin2)));
                    break;
                case MotorMode.Drive:
                    outData.d_out |= (ushort)(0b1 << ((byte)Pin1));
                    break;
                case MotorMode.ReverseDrive:
                    outData.d_out |= (ushort)(0b1 << ((byte)Pin2));
                    break;
            }
            return outData;
        }
        private void RotationImpl(double speed)
        {
            if (Math.Abs(speed) < 0.2)
            {
                Mode = MotorMode.Stop;
            }
            else if (speed > 0)
            {
                Mode = MotorMode.Drive;
            }
            else
            {
                Mode = MotorMode.ReverseDrive;
            }
        }
        public enum DigitalPin : byte
        {
            D0,D1, D2, D3, D4, D5,D6, D7
        }
    }
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
            MotorMode mode;
            if (Math.Abs(speedModifier) < 0.0001)
            {
                mode = MotorMode.Stop;
            }
            else if (speedModifier > 0)
            {
                mode = MotorMode.Drive;
            }
            else
            {
                mode = MotorMode.ReverseDrive;
            }
            speedModifier = Math.Clamp(speedModifier, 0, 1);
            HostDriver.AddMessage(new MotorDriver.Message(MotorID,mode, (byte)(speedModifier * 255)));
        }
        public void Brake()
        {
            HostDriver.AddMessage(new MotorDriver.Message(MotorID, MotorMode.Brake, 0));
        }
        public void Stop()
        {
            HostDriver.AddMessage(new MotorDriver.Message(MotorID, MotorMode.Stop, 0));
        }

        public override string ModuleName => "アレイニー モーター";

        public override string ModuleDescription => "アレイニーの汎用モータ";
    }
}

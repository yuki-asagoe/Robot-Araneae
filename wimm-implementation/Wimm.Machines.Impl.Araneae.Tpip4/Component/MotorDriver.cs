using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Wimm.Machines.TpipForRasberryPi.Import;

namespace Wimm.Machines.Impl.Araneae.Tpip4.Component
{
    internal class MotorDriver(byte i2cAddress)
    {
        public byte I2CAddress { get; } = i2cAddress;
        private Message?[] MessageBuffer { get; } = new Message?[4];

        public void AddMessage(Message message)
        {
            if (message.MotorID < 0 || message.MotorID >= 4) { return; }
            MessageBuffer[message.MotorID] = message;
        }
        public void SendAll()
        {
            var messages = new LinkedList<byte>();
            foreach(var m in MessageBuffer)
            {
                if(m is Message message)
                    foreach(var i in message.Construct())
                    {
                        messages.AddLast(i);
                    }
            }
            Array.Fill(MessageBuffer, null);
            var data = messages.ToArray();
            TPJT4.NativeMethods.Send_I2Cdata(0, data, I2CAddress, data.Length);
        }

        public record struct Message(byte MotorID,MotorMode Type,byte Speed=0)
        {
            public byte[] Construct()
            {
                if(Type == MotorMode.Stop || Type == MotorMode.Brake)
                {
                    return [(byte)((MotorID << 4) | ((byte)Type))];
                }
                else
                {
                    return [(byte)((MotorID << 4) | ((byte)Type)),Speed];
                }
            }
        }
        public enum MotorMode : byte
        {
            Stop=0,Brake,Drive,ReverseDrive
        }
    }
}

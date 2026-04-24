import React, { useEffect, useRef, useState } from "react";
import { LineChart, Line, YAxis, ResponsiveContainer } from "recharts";
import { Activity, Heart, Wind, Mic2, ShieldAlert, Wifi } from "lucide-react";
import { motion, AnimatePresence } from "framer-motion";
import { Card, CardContent } from "./components/ui/card";
import { Button } from "./components/ui/button";
import { Badge } from "./components/ui/badge";

// Giả lập sóng PQRST chuyên nghiệp
const generateECG = (t) => {
  const phase = (t % 1);
  const p = Math.exp(-Math.pow((phase - 0.2) / 0.05, 2)) * 0.2;
  const qrs = Math.exp(-Math.pow((phase - 0.4) / 0.01, 2)) * 1.5;
  const t_wave = Math.exp(-Math.pow((phase - 0.7) / 0.08, 2)) * 0.4;
  return p + qrs + t_wave + (Math.random() * 0.05);
};

export default function AdvancedDashboard() {
  const [data, setData] = useState(new Array(100).fill({ y: 0 }));
  const [heartRate, setHeartRate] = useState(75);
  const timeRef = useRef(0);

  useEffect(() => {
    const interval = setInterval(() => {
      timeRef.current += 0.02;
      const newVal = generateECG(timeRef.current);
      setData(prev => [...prev.slice(1), { y: newVal }]);
      // Ngẫu nhiên thay đổi nhịp tim một chút cho thật
      if (Math.random() > 0.95) setHeartRate(prev => prev + (Math.random() > 0.5 ? 1 : -1));
    }, 20);
    return () => clearInterval(interval);
  }, []);

  return (
    <div className="min-h-screen bg-black text-slate-100 p-4 md:p-8 font-mono">
      {/* Header */}
      <div className="max-w-7xl mx-auto flex justify-between items-center mb-8 border-b border-emerald-500/20 pb-4">
        <div>
          <h1 className="text-2xl font-black tracking-tighter flex items-center gap-2 text-emerald-400">
            <div className="w-3 h-3 bg-emerald-500 rounded-full animate-pulse" />
            VITAL-CORE AIoT v2.0
          </h1>
          <p className="text-xs text-slate-500 uppercase tracking-widest">Medical Intelligence Systems</p>
        </div>
        <div className="flex gap-4 items-center">
          <Badge variant="success" className="bg-emerald-500/10 text-emerald-400">
            <Wifi size={14} className="mr-1" /> DEVICE-01: CONNECTED
          </Badge>
          <div className="text-right text-xs">
            <p className="text-slate-500">SESSION TIME</p>
            <p className="font-bold">02:45:12</p>
          </div>
        </div>
      </div>

      <div className="max-w-7xl mx-auto grid grid-cols-12 gap-6">
        
        {/* Left Column: Big Stats */}
        <div className="col-span-12 lg:col-span-4 space-y-6">
          {/* Heart Rate Card */}
          <Card className="bg-zinc-950 border-zinc-800 relative overflow-hidden group">
            <div className="absolute top-0 left-0 w-1 h-full bg-red-600 shadow-[0_0_15px_rgba(220,38,38,0.5)]" />
            <CardContent className="p-6">
              <div className="flex justify-between items-start">
                <span className="text-zinc-500 text-sm font-bold">HEART RATE</span>
                <Heart className="text-red-600 animate-bounce" size={20} />
              </div>
              <div className="flex items-baseline gap-2 mt-2">
                <motion.span 
                  key={heartRate}
                  initial={{ opacity: 0, y: 10 }}
                  animate={{ opacity: 1, y: 0 }}
                  className="text-7xl font-black tracking-tighter text-white"
                >
                  {heartRate}
                </motion.span>
                <span className="text-red-500 font-bold italic">BPM</span>
              </div>
              <div className="mt-4 h-1 w-full bg-zinc-800 rounded-full overflow-hidden">
                <motion.div 
                  animate={{ width: ["20%", "80%", "20%"] }}
                  transition={{ duration: 2, repeat: Infinity }}
                  className="h-full bg-red-600 shadow-[0_0_10px_red]"
                />
              </div>
            </CardContent>
          </Card>

          {/* Respiration Card */}
          <Card className="bg-zinc-950 border-zinc-800 relative overflow-hidden">
            <div className="absolute top-0 left-0 w-1 h-full bg-sky-500 shadow-[0_0_15px_rgba(14,165,233,0.5)]" />
            <CardContent className="p-6">
              <div className="flex justify-between items-start">
                <span className="text-zinc-500 text-sm font-bold">RESPIRATION</span>
                <Wind className="text-sky-500" size={20} />
              </div>
              <div className="flex items-baseline gap-2 mt-2">
                <span className="text-6xl font-black tracking-tighter text-white">18</span>
                <span className="text-sky-500 font-bold italic">BR/M</span>
              </div>
            </CardContent>
          </Card>
        </div>

        {/* Right Column: Waveforms */}
        <div className="col-span-12 lg:col-span-8 space-y-6">
          <Card className="bg-zinc-950 border-zinc-800 h-[420px] relative">
            <div className="absolute inset-0 opacity-10 pointer-events-none" 
                 style={{backgroundImage: 'linear-gradient(#10b981 1px, transparent 1px), linear-gradient(90deg, #10b981 1px, transparent 1px)', backgroundSize: '40px 40px'}} 
            />
            <CardContent className="p-0 h-full">
              <div className="p-4 flex justify-between items-center border-b border-zinc-800">
                <div className="flex items-center gap-2">
                  <Activity className="text-emerald-500" size={18} />
                  <span className="text-xs font-bold tracking-widest text-emerald-500">LIVE ECG WAVEFORM (II)</span>
                </div>
                <span className="text-[10px] text-zinc-500">25mm/s | 10mm/mV</span>
              </div>
              <div className="h-[340px] w-full pt-10">
                <ResponsiveContainer width="100%" height="100%">
                  <LineChart data={data}>
                    <YAxis domain={[-0.2, 1.8]} hide />
                    <Line 
                      type="monotone" 
                      dataKey="y" 
                      stroke="#10b981" 
                      strokeWidth={3} 
                      dot={false} 
                      isAnimationActive={false}
                      className="drop-shadow-[0_0_8px_rgba(16,185,129,0.8)]"
                    />
                  </LineChart>
                </ResponsiveContainer>
              </div>
            </CardContent>
          </Card>
        </div>

        {/* Bottom Section: Audio Analysis */}
        <div className="col-span-12">
          <Card className="bg-zinc-900/50 border-emerald-500/20 backdrop-blur-md">
            <CardContent className="p-6 flex flex-col md:flex-row justify-between items-center gap-6">
              <div className="flex items-center gap-4">
                <div className="p-4 bg-red-500/10 rounded-full border border-red-500/20">
                  <Mic2 className="text-red-500 animate-pulse" />
                </div>
                <div>
                  <h4 className="font-bold text-white uppercase tracking-tight">Heart & Lung Sound Recorder</h4>
                  <p className="text-xs text-zinc-500 italic">Capturing high-frequency phonocardiogram signals...</p>
                </div>
              </div>
              <div className="flex gap-3">
                <Button className="bg-red-600 hover:bg-red-700 text-xs px-8 h-10 shadow-[0_0_15px_rgba(220,38,38,0.3)]">START RECORDING</Button>
                <Button variant="outline" className="text-xs px-8 h-10 border-zinc-700">LIBRARY</Button>
              </div>
              <div className="bg-black/40 px-6 py-2 rounded-lg border border-zinc-800 min-w-[200px]">
                <span className="text-[10px] text-zinc-500 block mb-1">AI ANALYTICS</span>
                <span className="text-emerald-400 font-bold text-sm tracking-widest">NORMAL RHYTHM</span>
              </div>
            </CardContent>
          </Card>
        </div>
      </div>
    </div>
  );
}
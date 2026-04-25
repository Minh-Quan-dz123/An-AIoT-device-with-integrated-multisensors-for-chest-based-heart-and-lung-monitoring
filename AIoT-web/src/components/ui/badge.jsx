import React from "react"

export const Badge = ({ children, variant = "success", className = "" }) => {
  const variants = {
    success: "bg-emerald-500/10 text-emerald-400 border-emerald-500/20 shadow-[inset_0_0_10px_rgba(16,185,129,0.1)]",
    outline: "border border-zinc-800 text-zinc-500 bg-zinc-900/50",
    danger: "bg-red-500/10 text-red-500 border-red-500/20 animate-pulse",
  }

  return (
    <span className={`inline-flex items-center px-3 py-1 rounded-full text-[10px] font-black border uppercase tracking-tighter transition-all duration-300 ${variants[variant]} ${className}`}>
      {children}
    </span>
  )
}
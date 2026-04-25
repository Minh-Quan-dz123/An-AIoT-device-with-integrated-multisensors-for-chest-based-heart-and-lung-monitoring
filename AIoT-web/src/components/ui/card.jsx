import React from "react"

export const Card = ({ children, className = "" }) => (
  <div className={`bg-zinc-950/80 border border-zinc-800 rounded-2xl backdrop-blur-md shadow-2xl overflow-hidden transition-all duration-300 hover:border-zinc-700 ${className}`}>
    {children}
  </div>
)

export const CardContent = ({ children, className = "" }) => (
  <div className={`p-6 ${className}`}>{children}</div>
)

export const CardHeader = ({ children, className = "" }) => (
  <div className={`px-6 py-4 border-b border-zinc-800/50 flex justify-between items-center ${className}`}>
    {children}
  </div>
)

export const CardTitle = ({ children, className = "" }) => (
  <h3 className={`text-xs font-bold tracking-[0.2em] text-zinc-500 uppercase ${className}`}>
    {children}
  </h3>
)
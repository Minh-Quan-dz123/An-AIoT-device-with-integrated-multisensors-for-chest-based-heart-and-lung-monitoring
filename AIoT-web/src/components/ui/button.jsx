import React from "react"

export const Button = ({ children, variant = "primary", className = "", ...props }) => {
  const baseStyles = "px-6 py-2 rounded-xl font-black uppercase tracking-widest text-[10px] transition-all active:scale-95 duration-200 cursor-pointer"
  
  const variants = {
    primary: "bg-red-600 text-white hover:bg-red-500 shadow-[0_0_15px_rgba(220,38,38,0.3)] hover:shadow-[0_0_25px_rgba(220,38,38,0.5)] border-t border-red-400/30",
    outline: "border border-zinc-700 text-zinc-400 hover:text-white hover:border-zinc-500 hover:bg-zinc-800",
  }

  return (
    <button className={`${baseStyles} ${variants[variant]} ${className}`} {...props}>
      {children}
    </button>
  )
}
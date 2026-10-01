const sliders=[1,2,3,4].map(n=>document.getElementById(`s${n}`));
const voltage=(p)=>2.5+(Number(p)/100)*1.8;

function classify(cells){
  const avg=cells.reduce((a,b)=>a+b,0)/4;
  const min=Math.min(...cells), max=Math.max(...cells);
  const imbalance=max-min;
  let health, healthCode, protection, protectionCode, relay, buzzer, healthText, protectionText;

  // Project demonstration thresholds:
  // <0.10 V: healthy; 0.10-<0.30 V: minor; 0.30-<0.60 V: critical; >=0.60 V: pack failure.
  // Voltage limits: 3.00 V minimum and 4.20 V maximum.
  const voltageFault=cells.some(v=>v<3.0 || v>4.2);

  if (voltageFault || imbalance>=0.60) {
    health="PACK FAILURE"; healthCode=3; protection="SHUTDOWN"; protectionCode=3;
    relay="OFF"; buzzer="ON"; healthText="A severe cell or pack-level fault requires shutdown."; protectionText="Pack operation is isolated.";
  } else if (imbalance>=0.30) {
    health="CRITICAL"; healthCode=2; protection="FAILSAFE"; protectionCode=2;
    relay="OFF"; buzzer="ON"; healthText="Critical cell imbalance detected."; protectionText="Protective isolation is active.";
  } else if (imbalance>=0.10) {
    health="MINOR IMBALANCE"; healthCode=1; protection="DEGRADED"; protectionCode=1;
    relay="ON"; buzzer="OFF"; healthText="Minor imbalance detected; operation continues in degraded mode."; protectionText="Monitoring continues with degraded protection state.";
  } else {
    health="HEALTHY"; healthCode=0; protection="NORMAL"; protectionCode=0;
    relay="ON"; buzzer="OFF"; healthText="All simulated cells are within the healthy operating range."; protectionText="Battery operation is allowed.";
  }
  return {avg,min,max,imbalance,health,healthCode,protection,protectionCode,relay,buzzer,healthText,protectionText};
}

function render(){
  const cells=sliders.map(s=>voltage(s.value));
  const r=classify(cells);
  cells.forEach((v,i)=>{
    document.getElementById(`v${i+1}`).textContent=v.toFixed(2)+" V";
    document.getElementById(`bar${i+1}`).textContent=v.toFixed(2)+" V";
    const pct=Math.max(0,Math.min(100,((v-2.5)/1.8)*100));
    document.getElementById(`fill${i+1}`).style.width=pct+"%";
  });
  document.getElementById("avg").textContent=r.avg.toFixed(2)+" V";
  document.getElementById("imb").textContent=r.imbalance.toFixed(2)+" V / "+((r.imbalance/r.avg)*100).toFixed(2)+"%";
  document.getElementById("weak").textContent="Cell "+(cells.indexOf(r.min)+1);
  document.getElementById("strong").textContent="Cell "+(cells.indexOf(r.max)+1);

  const health=document.getElementById("health"), protection=document.getElementById("protection");
  health.textContent=r.health; health.className="status "+({0:"healthy",1:"minor",2:"critical",3:"failure"}[r.healthCode]);
  protection.textContent=r.protection; protection.className="status "+({0:"normal",1:"degraded",2:"failsafe",3:"shutdown"}[r.protectionCode]);
  document.getElementById("healthText").textContent=r.healthText;
  document.getElementById("protectionText").textContent=r.protectionText;
  document.getElementById("healthCode").textContent=r.healthCode;
  document.getElementById("protectionCode").textContent=r.protectionCode;
  document.getElementById("relay").textContent=r.relay;
  document.getElementById("buzzer").textContent=r.buzzer;
}

sliders.forEach(s=>s.addEventListener("input",render));
document.getElementById("resetBtn").addEventListener("click",()=>{sliders.forEach(s=>s.value=50);render();});
render();

R"(<!doctype html>
<html lang=ru>
<head>
<meta charset=utf-8>
<meta name=viewport content="width=device-width,initial-scale=1">
<title>Контроллер gpio</title>
<style>body{margin:0;padding:10px 15px;font-family:Arial}body *{box-sizing:border-box}p{margin:0;padding:0}button{cursor:pointer}.services_tab{display:flex;justify-content:space-around;flex-wrap:wrap;margin-bottom:20px}.services_tab__detail{position:relative;line-height:inherit;text-align:center;border:none;background:0 0;font-size:16px;font-weight:700;line-height:inherit;letter-spacing:.2em;color:#000;text-transform:uppercase}@media (max-width:767px){.services_tab__detail{max-width:48%}}.services_tab__detail{width:160px;padding:10px 0}.services_tab__detail::before{content:"";position:absolute;top:14px;left:0;width:100%;height:8px;background-color:#f0c52e;z-index:-1}.services_tab__detail:hover{background-color:#f0c52e}.services_tab__detail:active{color:#f7e184;background-color:#000}.services .services_tab__detail.current{display:block;color:#f7e184;background-color:#000}.tab_content{margin:0 -15px;padding:15px;background-color:#e5e5e5}.status{display:grid;grid-template-columns:auto auto auto;align-items:flex-start;justify-content:space-between;grid-column-gap:40px;grid-row-gap:20px;max-width:800px;margin:0 auto}@media (max-width:600px){.status{grid-template-columns:100%}}li.changed{outline:2px solid #f0c52e}.lamps_list,.pins_list,.rules_list,.status__list{grid-row:1/4;margin:0;padding-left:0;list-style:none}.status__list li{margin-bottom:10px}.status__list li{display:grid;grid-template-columns:20px 1fr 50px;grid-column-gap:5px;align-items:center;min-height:28px}.status__list .disabled{color:gray}#status_in_list li{grid-template-columns:20px 1fr}.status__list_indicator{position:relative;width:20px;height:20px;font-size:0;border-radius:50%;border:2px solid #000;background-color:#fff}.status__list_indicator.active::before{content:'';position:absolute;top:2px;left:2px;width:12px;height:12px;border-radius:50%;background-color:green}.disabled .status__list_indicator,.status__list .disabled button{opacity:.3}.status__list button{padding:5px 0;text-align:center;font-size:15px;background-color:#f0c52e;border:none;box-shadow:2px 2px 0 rgba(0,0,0,.3)}.status__list button:active{box-shadow:none}.status__text{display:grid;grid-template-columns:auto auto;grid-column-gap:15px;grid-row-gap:3px;margin-left:auto;font-size:15px}.entry_list__header,.pins_list{max-width:500px;margin:0 auto}.entry_list__header,.pins_list li{margin-bottom:8px}.pins_list li{padding:3px}.entry_list__header,.entry_list__item{display:grid;grid-template-columns:20px 1fr 1fr 30px;grid-column-gap:15px;align-items:center}#ins_list .entry_list__item,.entry_list__header--4{grid-template-columns:20px 1fr 130px 1fr 30px}.entry_list__name{max-width:350px;width:100%;padding:3px 5px;line-height:16px;border:1px solid #464646;background-color:#fff}select{width:100%;padding:3px 5px;line-height:16px;border:1px solid #464646;background-color:#fff}.save_btn{width:30px;height:30px;font-size:0;background-color:transparent;border:none;background-image:url("data:image/svg+xml,%3Csvg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 407.096 407.096' style='enable-background:new 0 0 407.096 407.096;' xml:space='preserve'%3E%3Cpath d='M402.115,84.008L323.088,4.981C319.899,1.792,315.574,0,311.063,0H17.005C7.613,0,0,7.614,0,17.005v373.086 c0,9.392,7.613,17.005,17.005,17.005h373.086c9.392,0,17.005-7.613,17.005-17.005V96.032 C407.096,91.523,405.305,87.197,402.115,84.008z M300.664,163.567H67.129V38.862h233.535V163.567z'/%3E%3Cpath d='M214.051,148.16h43.08c3.131,0,5.668-2.538,5.668-5.669V59.584c0-3.13-2.537-5.668-5.668-5.668h-43.08 c-3.131,0-5.668,2.538-5.668,5.668v82.907C208.383,145.622,210.92,148.16,214.051,148.16z'/%3E%3C/svg%3E");background-repeat:no-repeat;background-size:30px;background-position:center}.save_btn:active,.save_btn:focus,.save_btn:hover{filter:invert(96%) sepia(46%) saturate(6340%) hue-rotate(330deg) brightness(98%) contrast(91%)}.settings{display:grid;grid-template-columns:repeat(2,1fr);grid-column-gap:30px;max-width:850px;margin:0 auto}@media (max-width:768px){.settings{grid-template-columns:100%}}.settings fieldset{margin:5px;border-color:#464646}.settings fieldset.changed{border-color:#f0c52e}.fieldset__common .settings__text{grid-template-columns:1fr 120px}.settings legend{padding:3px 5px;font-weight:700;font-size:15px;text-transform:uppercase;background-color:#f0c52e}.settings__check{display:flex;align-items:center;margin-bottom:5px}.settings__dhcp{display:grid;grid-column-gap:5px;grid-template-columns:60px auto}input[type=checkbox]{width:20px;height:20px;margin-left:10px;border-color:#464646}.settings__text{display:grid;grid-template-columns:60px 90px;align-items:center;grid-column-gap:15px;grid-row-gap:10px}.settings__text.input-big{grid-template-columns:60px 1fr}.settings .entry_list__btn{grid-column:1/-1;margin:15px auto}.settings label{font-size:15px;font-weight:700}.settings__btn{grid-column:1/-1;margin:15px auto;padding:5px;text-align:center;font-size:15px;background-color:#f0c52e;border-color:#f0c52e;border:1px solid #464646;cursor:pointer;display:block}.text_info{text-align:center;margin-top:5px;margin-bottom:15px;font-style:italic}.text_info span{font-style:normal;font-family:Georgia;color:#07074b}.entry_list__header{font-weight:700}</style>
</head>
<body>
<section class=services>
<div class=services_tab>
<button class=services_tab__detail data-target=manage>Управление</button>
<button class=services_tab__detail data-target=ins>Входы</button>
<button class=services_tab__detail data-target=outs>Выходы</button>
<button class=services_tab__detail data-target=settings>Настройки</button>
<button class=services_tab__detail data-target=mqtt_info>MQTT info</button>
</div>
<div class=tab_content id=manage>
<div class=status>
<ul class=status__list id=status_in_list></ul>
<ul class=status__list id=status_out_list></ul>
<div class=status__text>
<span>IP:</span><b id=status_ip></b>
<span>Mqtt:</span><b id=status_mqtt></b>
<span>Uptime (секунд):</span><b id=status_uptime></b>
<span>Версия:</span><b id=version></b>
</div>
</div>
</div>
<div class=tab_content id=ins>
<div class="entry_list__header entry_list__header--4">
<span></span>
<span>Имя</span>
<span>Задержка переключения</span>
<span>Используется</span>
</div>
<ol class=pins_list id=ins_list></ol>
</div>
<div class=tab_content id=outs>
<div class=entry_list__header>
<span></span>
<span>Имя</span>
<span>Используется</span>
</div>
<ol class=pins_list id=outs_list></ol>
</div>
<div class=tab_content id=settings>
<div class=settings>
<fieldset>
<legend>сеть</legend>
<div class="settings__check settings__dhcp">
<label>DHCP</label>
<input id=settings_net_dhcp type=checkbox>
</div>
<div class="settings__text input-big">
<label>ip</label> <input class=entry_list__name placeholder=192.168.0.180 id=settings_net_ip maxlength=15>
<label>маска подсети</label> <input class=entry_list__name placeholder=55.255.255.0 id=settings_net_mask maxlength=15>
<label>ip шлюза</label> <input class=entry_list__name placeholder=192.168.0.1 id=settings_net_gateway maxlength=15>
</div>
</fieldset>
<fieldset name=mqtt>
<legend>mqtt</legend>
<div class="settings__text input-big">
<label>сервер</label> <input class=entry_list__name id=settings_mqtt_server title="Доменное имя или ip адрес" maxlength=63>
<label>топик</label> <input class=entry_list__name id=settings_mqtt_topic maxlength=63>
</div>
</fieldset>
<button class=settings__btn id=settins_save_btn>Сохранить</button>
</div>
</div>
<div class=tab_content id=mqtt_info>
<fieldset>
<legend>Исходящие топики</legend>
<div class=text_info style=text-align:left>
<ul>
<li>
<b>&lt;топик&gt;/info</b> - информация по состоянию (json)<br>
Формат: <span>
{uptime: &lt;uptime&gt;, ip: "&lt;ip&gt;", ins: [&lt;in&gt;,&lt;in&gt;...], outs: [&lt;out&gt;,&lt;out&gt;...]}
</span><br>
in: <span>{name: "имя входа", enabled: 0|1, value: 0|1, throttle_time: &lt;0-65535&gt;}</span><br>
out: <span>{name: "имя выхода", enabled: 0|1, value: 0|1}</span>
</li>
<li>
<b>&lt;топик&gt;/in/&lt;имя входа&gt;</b> [retain] - состояние входа, <span>0 или 1</span>
</li>
<li>
<b>&lt;топик&gt;/out/&lt;имя выхода&gt;</b> [retain] - состояние выхода, <span>0 или 1</span>
</li>
</ul>
</div>
</fieldset>
<fieldset>
<legend>Входящие топики</legend>
<div class=text_info style=text-align:left>
<ul>
<li>
<b>&lt;топик&gt;/out/&lt;имя выхода&gt;/set</b> - изменить состояние выхода<br>
Формат: <span>0 или 1</span>
</li>
</ul>
</div>
</fieldset>
</div>
</section>
<script>
const url="",fieldsDataActionsMap={status_ip:["ip"],status_uptime:["uptime"],version:["version"],settings_mqtt_server:["mqtt_server",null,"/mqtt_set_server?server="],settings_mqtt_topic:["mqtt_topic",null,"/mqtt_set_topic?topic="],settings_net_ip:["static_ip",null,"/net_set_ip?ip="],settings_net_mask:["subnet_mask",null,"/net_set_subnet?ip="],settings_net_gateway:["gateway_ip",null,"/net_set_gateway_ip?ip="],settings_net_dhcp:["dhcp",null,"/net_set_dhcp?dhcp=",t=>t?1:0]};let data={};const $=document.querySelector.bind(document);async function getResource(t){let e=await fetch(t);if(!e.ok)throw new Error(`Could not fetch ${t}, status: ${e.status}`);const a=await e.json();return a.error&&alert(a.error),a}function updateDataInfoEls(){for(let[t,[e,a]]of Object.entries(fieldsDataActionsMap)){const n=document.getElementById(t),s=a?a(data[e]):data[e];n.type?"checkbox"==n.type?n.checked=s:n.value=s:n.innerHTML=s}}function detectAndShowChanges(t){t.querySelectorAll("select,input").forEach(e=>{e.addEventListener("change",()=>{t.classList.add("changed")})})}async function showManageTab(){data=await getResource(url+"/info"),updateDataInfoEls();const t=$("#status_mqtt");data.mqtt_connected?(t.innerText="подлючено",t.style.color="green"):(t.innerText="отключено",t.style.color="red");let e=$("#status_in_list");e.innerHTML="",data.ins.forEach((t,a)=>{const n=document.createElement("li");n.innerHTML=`<span class="status__list_indicator${t.value?" active":""}"></span><span>${t.name}</span>`,t.enabled||n.classList.add("disabled"),e.append(n)}),(e=$("#status_out_list")).innerHTML="",data.outs.forEach((t,a)=>{const n=document.createElement("li");n.innerHTML=`<span class="status__list_indicator${t.value?" active":""}"></span><span>${t.name}</span><button data-id="${a}" type="button">${t.value?"выкл":"вкл"}</button>`,t.enabled||n.classList.add("disabled"),e.append(n);const s=n.querySelector("button");s.addEventListener("click",async()=>{const e=n.querySelector(".status__list_indicator");s.disabled=!0;const i=await getResource(`${url}/out_set_value?id=${a}&val=${t.value?0:1}`);s.disabled=!1,i.error||(t.value?(e.classList.remove("active"),s.innerText="вкл",t.value=0):(e.classList.add("active"),s.innerText="выкл",t.value=1))})})}async function showInsListTab(){const t=$("#ins_list");t.innerHTML="",data.ins.forEach(({name:e,enabled:a,throttle_time:n},s)=>{const i=document.createElement("li");i.innerHTML=`\n\t\t\t\t<div class="entry_list__item">\n\t\t\t\t\t<div class="entry_list__counter">${s+1}.</div>\n\t\t\t\t\t<input data-id="name" class="entry_list__name" value="${e}" maxlength="30">\n\t\t\t\t\t<input data-id="throttle" value="${n}" type="number" min=0 max=65535>\n\t\t\t\t\t<input data-id="enabled" type="checkbox" ${a?"checked":""}>\n\t\t\t\t\t<button class="save_btn" title="Сохранить">Сохранить</button>\n\t\t\t\t</div>`,t.append(i),detectAndShowChanges(i);const c=i.querySelector('[data-id="name"]'),d=i.querySelector('[data-id="throttle"]'),l=i.querySelector('[data-id="enabled"]'),o=i.querySelector(".save_btn");o.addEventListener("click",async()=>{c.disabled=d.disabled=l.disabled=o.disabled=!0,await getResource(`${url}/in_change?id=${s}&throttle_time=${d.value}&name=${c.value}&enabled=${l.checked?1:0}`),data.ins[s].throttle_time=d.value,data.ins[s].name=c.value,data.ins[s].enabled=l.checked,c.disabled=d.disabled=l.disabled=o.disabled=!1,i.classList.remove("changed")})})}async function showOutsListTab(){const t=$("#outs_list");t.innerHTML="",data.outs.forEach(({name:e,enabled:a},n)=>{const s=document.createElement("li");s.innerHTML=`\n\t\t\t\t<div class="entry_list__item">\n\t\t\t\t\t<div class="entry_list__counter">${n+1}.</div>\n\t\t\t\t\t<input data-id="name" class="entry_list__name" value="${e}" maxlength="30">\n\t\t\t\t\t<input data-id="enabled" type="checkbox" ${a?"checked":""}>\n\t\t\t\t\t<button class="save_btn" title="Сохранить">Сохранить</button>\n\t\t\t\t</div>`,t.append(s),detectAndShowChanges(s);const i=s.querySelector('[data-id="name"]'),c=s.querySelector('[data-id="enabled"]'),d=s.querySelector(".save_btn");d.addEventListener("click",async()=>{i.disabled=c.disabled=d.disabled=!0,await getResource(`${url}/out_change?id=${n}&name=${i.value}&enabled=${c.checked?1:0}`),data.outs[n].name=i.value,data.outs[n].enabled=c.checked,i.disabled=c.disabled=d.disabled=!1,s.classList.remove("changed")})})}const dhcpEl=$("#settings_net_dhcp");function onChangeDhcp(){[$("#settings_net_ip"),$("#settings_net_mask"),$("#settings_net_gateway")].forEach(t=>t.disabled=dhcpEl.checked)}async function showSettingsTab(){data=await getResource(url+"/info"),updateDataInfoEls(),onChangeDhcp()}dhcpEl.addEventListener("click",onChangeDhcp),document.querySelectorAll("#settings fieldset").forEach(t=>detectAndShowChanges(t));const settinsSaveBtnEl=$("#settins_save_btn");function showActiveTab(t){const e=document.querySelectorAll(".tab_content"),a=document.getElementById(t);"manage"==t?showManageTab():"ins"==t?showInsListTab():"outs"==t?showOutsListTab():"settings"==t&&showSettingsTab(),e.forEach(t=>t.style.display="none"),a.style.display="block",document.querySelectorAll(".services_tab__detail").forEach(t=>t.classList.remove("current")),$(`[data-target="${t}"]`).classList.add("current")}settinsSaveBtnEl.addEventListener("click",async()=>{settinsSaveBtnEl.disabled=!0;for(let[t,[e,,a,n]]of Object.entries(fieldsDataActionsMap)){if(!a)continue;const s=document.getElementById(t);let i="checkbox"==s.type?s.checked:s.value;n&&(i=n(i)),i!=data[e]&&await getResource(url+a+i)}document.querySelectorAll("#settings fieldset").forEach(t=>t.classList.remove("changed")),await showSettingsTab(),settinsSaveBtnEl.disabled=!1,alert("Настройки успешно сохранены")}),document.querySelectorAll(".services_tab__detail").forEach(t=>{t.addEventListener("click",t=>{showActiveTab(t.target.getAttribute("data-target"))})}),showActiveTab("manage");
</script>
</body>
</html>)"

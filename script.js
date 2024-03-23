
let selected_perms = {}

function handle_click(event){
    const cell = event.target;
    const perm_attr = cell.attributes.getNamedItem("data-perm");
    if(perm_attr != null && perm_attr.value != ""){
        const perm = perm_attr.value;
        for(const other_cell of document.querySelectorAll("." + perm)){
            other_cell.classList.add("selected_elm");
        }
        selected_perms[perm] = true;
        for(sp in selected_perms){
            for(const crossing_cell of document.querySelectorAll("td.column_"+sp+".row_"+perm)){
                crossing_cell.style.backgroundColor = "";
                crossing_cell.classList.add("crossed_cell");
            }
            for(const crossing_cell of document.querySelectorAll("td.column_"+perm+".row_"+sp)){
                crossing_cell.style.backgroundColor = "";
                crossing_cell.classList.add("crossed_cell");
            }
        }
    }
}

function handle_hover(event, enter){
    const cell = event.target;
    let other_color = "grey";
    let this_color = "";
    let header_color = "#aa2222";

    if(!enter){
        other_color = "";
        this_color = "";
        header_color = "";
    }

    for(const row_or_column of ["row", "column"]){
        const attr = cell.attributes.getNamedItem("data-"+row_or_column);
        if(attr != null && attr.value != ""){
            for(const other_cell of document.querySelectorAll("td."+row_or_column+"_" + attr.value+(enter?":not(.crossed_cell)":""))){
                other_cell.style.backgroundColor = other_color;
            }
        }
    }
    cell.style.backgroundColor = this_color;

    const perm_attr = cell.attributes.getNamedItem("data-perm");
    if(perm_attr != null && perm_attr.value != ""){
        for(const other_cell of document.querySelectorAll("th." + perm_attr.value)){
            other_cell.style.backgroundColor = header_color;
        }
    }
}

function add_handlers(){
    for(const td of document.getElementsByTagName("td")){
        td.addEventListener("click",handle_click);
        td.addEventListener("mouseenter",(event)=>handle_hover(event,true));
        td.addEventListener("mouseleave",(event)=>handle_hover(event,false));
    }
    for(const th of document.getElementsByTagName("th")){
        th.addEventListener("click",handle_click);
        th.addEventListener("mouseenter",(event)=>handle_hover(event,true));
        th.addEventListener("mouseleave",(event)=>handle_hover(event,false));
    }
}

add_handlers();
